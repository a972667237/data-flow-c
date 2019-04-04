#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "threadpool.h"


#define INTSIZE 4
/*void va_start(va_list ap, argN);
type va_arg(va_list ap, type);
void va_copy(va_list dest, va_list src);
void va_end(va_list ap);

*/

struct DF_Ready_Flags;
struct DF_Fun_DC;
struct Active_Data;
struct DF_TargetADList;
struct DF_Fun_Node;
struct DF_TargetFunList;
struct DF_SourceItem;
typedef struct DF_Ready_Flags DF_Ready;
typedef struct DF_Fun_DC DF_DC;
typedef struct Active_Data DF_AD;
typedef struct DF_TargetADList DF_TADL;
typedef struct DF_Fun_Node DF_FN;
typedef struct DF_TargetFunList DF_TFL;
typedef struct DF_SourceItem DF_SI;

struct DF_Ready_Flags {
    int Flags;
    struct DF_Ready_Flags *next;
};

struct Active_Data{
    int Data_Count;					//用于记录链上有多少个数据对象可用    00 缓冲区数量   单位/个
    int Sum;                    //至今有过多少数据量           单位/个
    void *Data;
	int   Head;               //头尾 单位/个
	int   Tail;
    // int * ?
    int   MaxSize;
	int   persize;//数据单位大小             单位字节       size+4
    int FanOut;                      //指向几个目标函数	
    // 这里耶应该用函数指针列表吧
    DF_FN **DF_Fun_Index;           //指向目标数据流函数的 数据块二级指针      指向指针的指针
    // 指向数据流函数的数据，目前来看 Active Data 应该是唯一的
    int*   DF_flag_Index;          //是第几位
    pthread_rwlock_t lock;
};//主动数据链路

struct DF_TargetADList{
    int TargetNum;           //输入/输出主动数据数目
    DF_AD * Target[];   // 输入/输出主动数据指针 线性表
};
// 函数节点指向这里
// 存储输入输出数据
// 可是，如何进行分配，用 malloc 做动态？
// 记得看看 DF AD 是否有失效标志
// 看了下其实是对于每个函数，输入输出的总数据，都是一个链表，而 Target 存储了 形如 [*a, *b] n类数据，每一类都是一个链表，链一列数据，链的数据是 DF_AD*(Active_Data link list)

struct DF_Fun_Node {
	//DF_FN		* next_fun;						//构成线性链表
	//DF_FN		* next_ready_on_CPU;				//挂入某个CPU的就绪队列
	//DF_sched_data	sched_data;					//调度数据
	//DF_DAG_node 	DAG_node;						//数据流图节点
	void 		(*Func)(void);						//数据流函数
	unsigned 		char * Name;					//函数名	
    DF_Ready*       ready;
    pthread_mutex_t ready_lock;
	int             FinishNum;                        //当前完成了几个工作，初始为0   用以从数据端获得正确数据
    pthread_rwlock_t finish_lock;
	DF_TADL*      DF_Fun_AD_InPut;							//输入主动数据块
    // 每次运行时都要从这取数据吗？
    // 数据分配是怎样一个操作呢
	DF_TADL*      DF_Fun_AD_OutPut;                      //输出主动链块
    // 塞数据到里面
};
// 对于每个函数都创建一个这个

struct DF_TargetFunList{
    int Num;         //输出目标的数据流节点数
    DF_FN **Target;      //输出目标的数据流函数指针线性表
};   //线性表结构

struct DF_SourceItem{
    int stop;
    DF_FN* F;
};



DF_TFL DF_TFL_TABLE;
threadpool_t *pool;
DF_SI* source_list;
int source_list_len;

// DF_AD  c1_ad,c2_ad,a1_ad,b1_ad;//编译器加入
// comment it
DF_AD DF_source_ad, a_ad, b_ad, c_ad, d_ad, DF_output_ad;

// DF_FN C,X,Y,Z;//编译器加入
// comment it
DF_FN SOURCE_A_FN, DF_Source_FN, A_FN, B_FN, C_FN, D_FN;

// 主动数据链路初始化
void DF_ADInit(DF_AD* AD, int persize,int FanOut) {
    AD->Data_Count = 0; // 计数器为 0
    AD->Sum = 0;        // 总数据量为 0
    AD->Data = (void*)malloc(15 * (persize + INTSIZE)); // 初始化空间大小 15 个元素，persize 为数据单位大小(不记录数据调用次数，单纯的简单数据结构或者结构体)，而 INTSIZE 为其记录数（比如可调用次数）大小
    AD->Head = 0; // 好像是用循环队列存的，所以保存头尾，每次出头
    AD->Tail = 0;
    AD->MaxSize = 15; // 满队元素
    AD->persize = persize + INTSIZE; // persize 为最终大小，包括数据单位大小和一个计数器
    AD->FanOut = 0;//？ 这里应该是函数数目计数
    pthread_rwlock_init(&AD->lock, NULL);
    AD->DF_Fun_Index = (DF_FN**)malloc(sizeof(DF_FN*) * FanOut);   // 这个就是直接指向作为源数据的函数
    AD->DF_flag_Index = (int*)malloc(sizeof(int) * FanOut);   // 第几位这个用途待定
}

//FN>初始化函数一，  连接输入和FN节点的关系（FN指向AD，AD指向FN）
void DF_FNInit1(DF_FN* FN,void*FunAddress ,char *Name, int InPutADNum,...){
    int Flaglen;
    va_list ap;
    va_start(ap,InPutADNum);
    FN->DF_Fun_AD_InPut = (DF_TADL*)malloc(sizeof(DF_TADL) + InPutADNum * sizeof(DF_AD*));  //柔性数组使用的指针数组 可能有BUG	
    FN->DF_Fun_AD_InPut->TargetNum = InPutADNum;
    Flaglen = (InPutADNum + 256 - 1) / 256; //InPutADNum/256 向上取整;
    for(int i = 0;i < InPutADNum;i++){
        DF_AD* p;
        p = va_arg(ap, DF_AD*);
        FN->DF_Fun_AD_InPut->Target[i] = p;     //Input指向AD
        p->DF_Fun_Index[p->FanOut] = (DF_FN*)FN;       //AD指向FUN
        p->DF_flag_Index[p->FanOut] = i;      //AD是FN的第几位标志
        p->FanOut++;
    }
    FN->Func = (void (*)(void))FunAddress;   //初始化FN的一些常量信息，函数名，函数指针，就绪标志
    FN->Name = (unsigned char *)Name;
    FN->FinishNum = 0;
    FN->ready = 0;
    pthread_rwlock_init(&FN->finish_lock, NULL);
    pthread_mutex_init(&FN->ready_lock, NULL);
    va_end(ap);
}

//FN初始化函数二，连接FN节点和输出AD关系
void DF_FNInit2(DF_FN *FN, int OutPutADNum, ...){
    va_list ap;
    va_start(ap,OutPutADNum);
    FN->DF_Fun_AD_OutPut = (DF_TADL*)malloc(sizeof(DF_TADL) + OutPutADNum * sizeof(DF_AD*));  //柔性数组使用的指针数组 可能有BUG	
    FN->DF_Fun_AD_OutPut->TargetNum = OutPutADNum;
    for(int i = 0;i < OutPutADNum;i++){	
        FN->DF_Fun_AD_OutPut->Target[i] = va_arg(ap,DF_AD*);
    }
    va_end(ap);
}

void DF_AD_UpData(DF_FN *F,...){       //地址，地址，地址
	va_list ap;
	va_start(ap,F);
    void *new_data_addr; 
    int new_data_size;
	DF_AD *b;
	void *c;
	int Num=(F->DF_Fun_AD_OutPut->TargetNum);//一个函数输出的AD数量
	for(int i=0;i<Num;i++  ){
       new_data_addr = va_arg(ap, void*);//新数据地址
       new_data_size = va_arg(ap, int);
	   b = F->DF_Fun_AD_OutPut->Target[i];   //指向要跟新的AD
       pthread_rwlock_wrlock(&b->lock);
       if(b->MaxSize == b->Data_Count) {   //扩容
           void *p=(void*)malloc(b->MaxSize*b->persize*2);
           memcpy(p, (char*)b->Data + b->Head, b->MaxSize - b->Head);
           memcpy((char*)p + b->Head, b->Data, b->Head);	   
           free(b->Data);
           b->Data=p;
           b->Head=0;
           b->Tail=b->MaxSize;
           b->MaxSize=b->MaxSize*2;
       }
       memcpy(b->Data + b->persize * b->Tail, new_data_addr, new_data_size);
       memcpy(b->Data + b->persize * b->Tail + new_data_size, &b->FanOut, INTSIZE);

	   b->Tail = (b->Tail + 1) % b->MaxSize; // 还需要取模

	   b->Data_Count ++;
	   b->Sum ++;                  //跟新数据
       DF_FN *temp_f;
       DF_Ready *temp_r, *new_r;
       int done = 0;
       int flag;
       int finish_data_count;
       int finish_flag;
       for (int i=0; i < b->FanOut; i++) {
           done = 0;
           new_r = NULL;
           temp_f = b->DF_Fun_Index[i];
           pthread_mutex_lock(&temp_f->ready_lock);
           finish_data_count = temp_f->DF_Fun_AD_InPut->TargetNum; // 当前方法需要多少个数据
           finish_flag = (1 << finish_data_count) - 1; // 2 => 0000 0011
           flag = b->DF_flag_Index[i]; // 当前方法的第几个数据
           temp_r = temp_f->ready;
           while(temp_r) {
               if ((temp_r->Flags >> flag) & 1) {
                   temp_r = temp_r->next;
                   continue;
               }
               else {
                   temp_r->Flags = temp_r->Flags | (1 << flag);
                   done = 1;
                   break;
               }
           }

           temp_r = temp_f->ready;
           if (!done) {
               new_r = (DF_Ready*)malloc(sizeof(DF_Ready));
               new_r->Flags = 1 << flag;
               new_r->next = 0;
               if (temp_r) {
                   while(temp_r->next) {
                       temp_r = temp_r->next;
                   }
                   temp_r->next = new_r;
               } else {
                   temp_f->ready = new_r;
               }
               // add to tail
           }

           // judge that whather the function can run
           temp_r = temp_f->ready;
           printf("%d\n", temp_r->Flags);
           if(temp_r->Flags == finish_flag) {
               // if now flag == finish
               // fixme: not consider that if the queue is full
               threadpool_add(pool, (void (*)(void *))(temp_f->Func), NULL, 0);
               // consider that it must full in head, so if fail, all fail
               temp_f->ready = temp_r->next;
               free(temp_r);
           }
           pthread_mutex_unlock(&temp_f->ready_lock);
       }	
       pthread_rwlock_unlock(&b->lock);
   }
   va_end(ap);
}

/*
 * Method: DF_AD_GetData
 * Input: 
 *   - F: DF_FN, may get function data and count
 *   - ...:
 *     - Data: void?, may a address link to ad
 *     - Size: Data size
 *
 * */
void DF_AD_GetData(DF_FN* F, ...) {
    va_list ap;
    va_start(ap, F);

    void *data_addr;
    int data_size;

    int data_count = (F->DF_Fun_AD_InPut->TargetNum);

    DF_AD *temp_ad;
    int rest_count;
    pthread_rwlock_wrlock(&F->finish_lock);
    int finish_num = F->FinishNum;
    F->FinishNum ++;
    pthread_rwlock_unlock(&F->finish_lock);

    for (int i=0; i<data_count; i++) {
        data_addr = va_arg(ap, void*);
        data_size = va_arg(ap, int);

        temp_ad = F->DF_Fun_AD_InPut->Target[i];
        pthread_rwlock_rdlock(&temp_ad->lock);

        memcpy(data_addr, temp_ad->Data + (finish_num % temp_ad->MaxSize) * temp_ad->persize, data_size); // copy data

        // should make sure that temp_ad's data only operated by one thread

        // update list start
        memcpy(&rest_count, temp_ad->Data + (finish_num % temp_ad->MaxSize) * temp_ad->persize + data_size, INTSIZE);
        pthread_rwlock_unlock(&temp_ad->lock);
        pthread_rwlock_wrlock(&temp_ad->lock);
        if (rest_count > 1) {
            // rest more than 1, multi it
            rest_count = rest_count - 1;
            memcpy(temp_ad->Data + temp_ad->Head * temp_ad->persize + data_size, &rest_count, INTSIZE);
        } else {
            // rest is one, clean
            temp_ad->Head = (temp_ad->Head + 1) % temp_ad->MaxSize;
            temp_ad->Data_Count--;
        }
        pthread_rwlock_unlock(&temp_ad->lock);
        // update list end
    }

    va_end(ap);
}

void DF_SOURCE_Get_And_Update(DF_FN* F, int* count) {
    pthread_rwlock_wrlock(&F->finish_lock);
    *count = F->FinishNum;
    F->FinishNum ++;
    pthread_rwlock_unlock(&F->finish_lock);
}

void DF_Init(int InputFNNum, ...) {
    va_list ap;
    va_start(ap, InputFNNum);

    DF_TFL_TABLE.Num = InputFNNum;
    DF_TFL_TABLE.Target = (DF_FN**)malloc(sizeof(DF_FN*) * InputFNNum);

    for (int i=0; i<InputFNNum; i++) {
        DF_TFL_TABLE.Target[i] = va_arg(ap, DF_FN*);
    }

    va_end(ap);
}

void DF_Source_Init(void* source_data_addr, int datasize, int elementcount) {
    int defaultcount = DF_source_ad.FanOut;
    // 数据赋值可能存在性能问题
    if (DF_source_ad.MaxSize < elementcount) {
        // because ... only get data for one time
        // so can only alloc suitable space
        free(DF_source_ad.Data);
        DF_source_ad.Data = (void*)malloc(DF_source_ad.persize*elementcount);
        DF_source_ad.MaxSize = elementcount;
    }
    int flag;
    DF_FN *F;
    DF_Ready *new_r, *temp_r;
    int done;
    // default init
    for (int i=0; i<elementcount; i++) {
        memcpy(DF_source_ad.Data + DF_source_ad.persize * i, source_data_addr + datasize * i, datasize);

        memcpy(DF_source_ad.Data + DF_source_ad.persize * i + datasize, &defaultcount, INTSIZE);

        // try to trigger
        for(int i=0; i < DF_source_ad.FanOut; i++){
            done = 0;
            F = DF_source_ad.DF_Fun_Index[i];
            flag = DF_source_ad.DF_flag_Index[i];
            if (F->DF_Fun_AD_InPut->TargetNum == 1) {
                // trigger and not add
                threadpool_add(pool, (void (*)(void *))(F->Func), NULL, 0);
            } else {
                // add and not trigger
                temp_r = F->ready;
                while(temp_r) {
                    if ((temp_r->Flags >> flag) & 1) {
                        continue;
                    } else {
                        temp_r->Flags = temp_r->Flags | (1 << flag);
                        done = 1;
                    }
                    temp_r = temp_r->next;
                }
                // add to tail
                temp_r = F->ready;
                if (!done) {
                    new_r = (DF_Ready*)malloc(sizeof(DF_Ready));
                    new_r->Flags = 1 << flag;
                    new_r->next = NULL;
                    if (temp_r) {
                        while(temp_r->next) {
                            temp_r = temp_r->next;
                        }
                        temp_r->next = new_r;
                    } else {
                        F->ready = new_r;
                    }
                }
            }
        }
    }
    DF_source_ad.Tail = elementcount - 1;

}

void DF_Thread_Init(int threadnum, int queuesize) {
    if (!pool)
        pool = threadpool_create(threadnum, queuesize, 0);
}

void DF_SourceInit(int sourcenum, ...) {
    va_list ap;
    va_start(ap, sourcenum);
    DF_FN* source_fn_addr;

    source_list_len = sourcenum;
    source_list = (DF_SI*)malloc(sizeof(DF_SI) * sourcenum);

    for (int i=0; i<sourcenum; i++) {
        source_fn_addr = va_arg(ap, DF_FN*);
        source_list[i].F = source_fn_addr;
        source_list[i].stop = 0;
    }
}

int DF_Should_Stop() {
    // queue is 0
    // in run count is 0 (mean that not task run)
    // all source is stop
    int all_stop = 1;
    for (int i=0; i<source_list_len; i++) {
        if(source_list[i].stop == 0) {
            all_stop = 0;
            break;
        }
    }
    return threadpool_is_idle(pool)  && all_stop;
}

void DF_Loop() {
    // will replace with get from a list to use many source
    while(1) {
        if (DF_Should_Stop()) {
            break;
        }
        for (int i=0; i<source_list_len; i++) {
            if (!source_list[i].stop) {
                threadpool_add(pool, (void (*)(void *))(source_list[i].F->Func), NULL, 0);
            }
        }
        sleep(5);
    }
}

void DF_Destory() {
}

int* DF_Result() {
    return NULL;
}

void DF_Run (void* source_data_addr, int datasize, int elementcount) {
    DF_Thread_Init(5, 64);
    DF_Loop();
   // DF_Source_Init(source_data_addr, datasize, elementcount); // fixed by user
    DF_Destory();
}

void DF_Source_Stop(int item_index) {
    source_list[item_index].stop = 1;
}

void SOURCEA() {
    // need to get count and update finish count
    
    int DF_count;  // default info
    int DF_source;  // output data

    DF_SOURCE_Get_And_Update(&SOURCE_A_FN, &DF_count);

    {
        DF_source = DF_count * 2 + 1;

        if(DF_count == 10) {
            DF_Source_Stop(0);
        }
    }
    
    DF_AD_UpData(&SOURCE_A_FN, &DF_source, sizeof(DF_source));
}

void FUNA() {
    int DF_source;// compiler should guide that the type, maybe int?
    int a, b; // output data

    DF_AD_GetData(&A_FN, &DF_source, sizeof(DF_source)); // get data

    {
        // funA
        a = DF_source + 1;
        b = DF_source + 2;
        printf("a: -- %d\n", a);
        printf("b: -- %d\n", b);
    }

    DF_AD_UpData(&A_FN, &a, sizeof(a), &b, sizeof(b));
}

void FUNB() {
    int a; // get from A
    int c; // output data

    DF_AD_GetData(&B_FN, &a, sizeof(a));
    
    {
        // funB
        c = a + 1;
        printf("c: -- %d\n", c);
    }

    DF_AD_UpData(&B_FN, &c, sizeof(c));
}

void FUNC() {
    int b; // get from A
    int d; // output data

    DF_AD_GetData(&C_FN, &b, sizeof(b));

    {
        // func
        d = b + 1;
        printf("d: -- %d\n", d);
    }

    DF_AD_UpData(&C_FN, &d, sizeof(d));
}

void FUND() {
    int c, d; // get from B, C
    int e;
    
    DF_AD_GetData(&D_FN, &c, sizeof(c), &d, sizeof(d));

    {
        // funD
        e = c + d;
        printf("e: -- %d\n", e);
    }

    DF_AD_UpData(&D_FN, &e, sizeof(e));
  
}




//void FUNA(/*int x;;int a[]*/)
//{
//a[]=.......;
//DF_DF_update(DF_AD_eff);
//
//
//}
//
//void FUNC(void)    //a1,b1 是输入  ，c1,c2是输出 
//{
//  int a1=*(int*)Data;//编译器生成。
//  char b1=*(char*)Data;//赋值方式
//  
//  char c1,c2[5];//编译器生成
//  
//  DF_AD_GetData(&C,&a1,sizeof(a1),&b1,sizeof(b1))
//
//  
//  
//
//
//  	{/* 函数计算内容        
//       
//    c1=......
//    c2=.....    //由使用者填写
//
//  	*/}
//  
//
//   //两种更新数据方法
//
//   *(char*)(c1_ad.Data->Data + c1_ad.Data->used)=c1  ;//编译器生成 赋值方式
//
//
//   DF_AD_Updata(&C, &c1,sizeof(c1),&c2,sizeof(c2));//编译器生成       调用memcpy给数据块赋值  可以对数组变量也完成传递
//
//
//  
//
//
//}


int main() {
    DF_ADInit(&DF_source_ad, 4, 1);
    DF_ADInit(&a_ad, 4, 1);
    DF_ADInit(&b_ad, 4, 1);
    DF_ADInit(&c_ad, 4, 1);
    DF_ADInit(&d_ad, 4, 1);
    DF_ADInit(&DF_output_ad, 4, 1);
    // init by compiler

    DF_FNInit1(&SOURCE_A_FN, &SOURCEA, "SOURCEA", 0);
    DF_FNInit2(&SOURCE_A_FN, 1, &DF_source_ad);

    DF_FNInit1(&A_FN, &FUNA, "FUNA", 1, &DF_source_ad);
    DF_FNInit2(&A_FN, 2, &a_ad, &b_ad);

    DF_FNInit1(&B_FN, &FUNB, "FUNB", 1, &a_ad);
    DF_FNInit2(&B_FN, 1, &c_ad);

    DF_FNInit1(&C_FN, &FUNC, "FUNC", 1, &b_ad);
    DF_FNInit2(&C_FN, 1, &d_ad);

    DF_FNInit1(&D_FN, &FUND, "FUND", 2, &c_ad, &d_ad);
    DF_FNInit2(&D_FN, 1, &DF_output_ad);

    DF_SourceInit(1, &SOURCE_A_FN);
    // init by compiler

    DF_Init(4, &A_FN, &B_FN, &C_FN, &D_FN);

    // source head
    int a[20] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39};
    // source head end

    DF_Run(a, sizeof(int), 20); // run and destory data
    int *b = DF_Result();
    
    // source tail
    printf("%d", *a);
    return 0;
    // source tail end
}


//int main(){
//
//
//DF_ADInit(&c1_ad,4,1);
//DF_ADInit(&c2_ad,4,2);
//DF_ADInit(&a1_ad,4,3);
//DF_ADInit(&b1_ad,40,4);
//
//
//DF_FNInit1(&C, &FUNC, 1, "FUNC", 2,&a1_ad,&b1_ad);//编译器生成
//DF_FNInit2(&C, 2, &c1_ad,&c2_ad);  //编译器生成
//
//DFInit(&C, &FUNX,)
//
//
//
//
//DF_Init;//初始化线程
//DF_start;//开始执行
//
//
//
//DF_END;   //释放内存，清理空间  结束数据流函数
//
//
//
//
//}





//DF_DF_update(){
//FD_Activate;
//DF_stimulate();
//
//
//DF_consume();
//
//
//
//}









/*
#include <stdio.h>

#define FUNC_MAX(type) \
type max_##type(type a, type b)\
{\
if(a>b)\
return a;\
else\
return b;\
}

FUNC_MAX(int)
FUNC_MAX(float)

int main()
{
int n1,n2;
float f1,f2;

printf("Input two int:");
scanf("%d%d",&n1, &n2);
printf("max=%d\n", max_int(n1,n2));

printf("Input two float:");
scanf("%f%f",&f1, &f2);
printf("max=%f\n", max_float(f1,f2));

return 0;
}


*/

