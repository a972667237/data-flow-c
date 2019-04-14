#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "threadpool.h"
#include "dfc.h"

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

struct DF_TargetADList{
    int TargetNum;           //输入/输出主动数据数目
    DF_AD * Target[];   // 输入/输出主动数据指针 线性表
};

struct DF_SourceItem{
    int stop;
    DF_FN* F;
};

void DF_ADInit(DF_AD* AD, int persize,int FanOut) {
    AD->Data_Count = 0; // 计数器为 0
    AD->Sum = 0;        // 总数据量为 0
    AD->Data = (void*)malloc(15 * (persize + INTSIZE)); // 初始化空间大小 15 个元素，persize 为数据单位大小(不记录数据调用次数，单纯的简单数据结构或者结构体)，而 INTSIZE 为其记录数（比如可调用次数）大小
    AD->Head = 0; // 好像是用循环队列存的，所以保存头尾，每次出头
    AD->Tail = 0;
    AD->MaxSize = 15; // 满队元素
    AD->persize = persize + INTSIZE; // persize 为最终>大小，包括数据单位大小和一个计数器
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
        p->DF_flag_Index[p->FanOut] = i;      //AD是FN>的第几位标志
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

void DF_AD_UpData(DF_TFL *table, DF_FN *F,...){       //地址，地址，地址
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

       b->Tail = (b->Tail + 1) % b->MaxSize; // 还需要>取模

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
           flag = b->DF_flag_Index[i]; // 当前方法的第>几个数据
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
           if(temp_r->Flags == finish_flag) {
               // if now flag == finish
               // fixme: not consider that if the queue is full
               threadpool_add(table->pool, (void (*)(void *))(temp_f->Func), NULL, temp_f->item_index);
               printf_thread_info(table);
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

void DF_Init(DF_TFL* table, int InputFNNum, ...) {
    va_list ap;
    va_start(ap, InputFNNum);

    table->Num = InputFNNum;
    table->Target = (DF_FN**)malloc(sizeof(DF_FN*) * InputFNNum);
    table->pool = NULL;

    table->Func_Target = (void (**) (void *))malloc(sizeof(void (*) (void *)) * InputFNNum);
    table->should_hash = 1;

    for (int i=0; i<InputFNNum; i++) {
        table->Target[i] = va_arg(ap, DF_FN*);
        table->Target[i]->item_index = i;
        table->Func_Target[i] = (void (*) (void *))table->Target[i]->Func;
    } 

    va_end(ap);
}

void DF_Thread_Init(DF_TFL* table, int threadnum, int queuesize) {
    if (!table->pool){
        table->thread_num = threadnum;
        table->pool = threadpool_create(threadnum, queuesize, 0);
    }
}

void DF_SourceInit(DF_TFL *table, int sourcenum, ...) {
    va_list ap;
    va_start(ap, sourcenum);
    DF_FN* source_fn_addr;

    table->source_list_len = sourcenum;
    table->source_list = (DF_SI*)malloc(sizeof(DF_SI) * sourcenum);
    table->item_index_order = (int*)malloc(sizeof(int) * sourcenum);

    for (int i=0; i<sourcenum; i++) {
        source_fn_addr = va_arg(ap, DF_FN*);
        table->source_list[i].F = source_fn_addr;
        table->source_list[i].stop = 0;
        table->item_index_order[i] = i;
    }
}

void DF_OutputInit(DF_TFL *table, int outputnum, ...) {
    va_list ap;
    va_start(ap, outputnum);

    table->output_list_len = outputnum;
    table->output_list = (DF_AD**)malloc(sizeof(DF_AD*) * outputnum);

    for (int i=0; i<outputnum; i++) {
        table->output_list[i] = va_arg(ap, DF_AD*);
    }
}

int DF_Should_Stop(DF_TFL* table) {
    // queue is 0
    // in run count is 0 (mean that not task run)
    // all source is stop
    int all_stop = 1;
    for (int i=0; i < table->source_list_len; i++) {
        if(table->source_list[i].stop == 0) {
            all_stop = 0;
            break;
        }
    }
    return threadpool_is_idle(table->pool)  && all_stop;
}

void printf_thread_info(DF_TFL* table) {
    system("clear");

    for (int i=0; i<table->thread_num; i++) {
        if (table->thread_task[i] != -1)
            printf("Thread: %d, %s\n", i, table->Target[table->thread_task[i]]->Name);
        else
            printf("Thread: %d, NULL\n", i);
    }

    for (int i=0; i<table->Num; i++) {
        printf("Func: %s, Count: %d\n", table->Target[i]->Name, table->Target[i]->FinishNum);
    }

    int count;
    int** index;
    index = (int**)malloc(sizeof(int*));
    queue_info(table->pool, &count, index);
    
    for (int i=0; i<count; i++) {
        printf("%d\n", (*index)[i]);
        printf("Queue: %d, %s\n", i, table->Target[(*index)[i]]->Name);
        
    }
}

void DF_Loop(DF_TFL* table) {
    // will replace with get from a list to use many source
    int queue_count;
    table->thread_task = get_thread_info_addr(table->pool);
    while(1) {
        if (DF_Should_Stop(table)) {
            break;
        }
        queue_count = threadpool_queue_count(table->pool);
        
        if (queue_count < table->source_list_len) {
            for (int i=0; i<table->source_list_len; i++) {
                if (!table->source_list[i].stop) {
                    threadpool_add(table->pool, (void (*)(void *))(table->source_list[i].F->Func), NULL, table->source_list[i].F->item_index);
                    printf_thread_info(table);
                }
            }
        }

        if (queue_count > 2) {
            order_by_item_and_hash(table->pool, table->Func_Target, table->item_index_order, table->source_list_len, table->should_hash);

        }

    }
}

void DF_Destory() {
}

int* DF_Result() {
    return NULL;
}

void DF_Run (DF_TFL *table) {
    DF_Thread_Init(table, 10, 64);
    DF_Loop(table);
   // DF_Source_Init(source_data_addr, datasize, elementcount); // fixed by user
    DF_Destory();
}

void DF_Source_Stop(DF_TFL *table, int item_index) {
    table->source_list[item_index].stop = 1;
}
