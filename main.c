#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "threadpool.h"
#include "dfc.h"


DF_TFL DF_TFL_TABLE;

// DF_AD  c1_ad,c2_ad,a1_ad,b1_ad;//编译器加入
// comment it
//DF_AD DF_source_ad, a_ad, b_ad, c_ad, d_ad, DF_output_ad;
DF_AD DF_source_ad, DF_source_ad_b, DF_output_ad;

// DF_FN C,X,Y,Z;//编译器加入
// comment it
//DF_FN SOURCE_A_FN, DF_Source_FN, A_FN, B_FN, C_FN, D_FN;
DF_FN SOURCE_A_FN, SOURCE_B_FN, S_FN;

void SOURCEA() {
    int DF_count;
    int DF_source;

    DF_SOURCE_Get_And_Update(&SOURCE_A_FN, &DF_count);

    {
        DF_source = DF_count;

        if(DF_count == 10) {
            DF_Source_Stop(&DF_TFL_TABLE, 0);
        }
    }
    DF_AD_UpData(&DF_TFL_TABLE, &SOURCE_A_FN, &DF_source, sizeof(DF_source));
}

void SOURCEB() {
    int DF_count;
    int DF_source_B;

    DF_SOURCE_Get_And_Update(&SOURCE_B_FN, &DF_count);

    {
        DF_source_B = 100 - DF_count;
        printf("b: %d\n", DF_source_B);

        if (DF_count == 10) {
            DF_Source_Stop(&DF_TFL_TABLE, 1);
        }
    }
    DF_AD_UpData(&DF_TFL_TABLE, &SOURCE_B_FN, &DF_source_B, sizeof(DF_source_B));
}

void FUNS() {
    int a, b;
    int e;

    DF_AD_GetData(&S_FN, &a, sizeof(a), &b, sizeof(b));

    {
        // fun S
        e = a + b;
        printf("total: -- %d\n", e);
    }

    DF_AD_UpData(&DF_TFL_TABLE, &S_FN, &e, sizeof(e));
}
/*
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
    
    DF_AD_UpData(&DF_TFL_TABLE, &SOURCE_A_FN, &DF_source, sizeof(DF_source));
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

    DF_AD_UpData(&DF_TFL_TABLE, &A_FN, &a, sizeof(a), &b, sizeof(b));
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

    DF_AD_UpData(&DF_TFL_TABLE, &B_FN, &c, sizeof(c));
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

    DF_AD_UpData(&DF_TFL_TABLE, &C_FN, &d, sizeof(d));
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

    DF_AD_UpData(&DF_TFL_TABLE, &D_FN, &e, sizeof(e));
  
}
*/
int main() {
    /*
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
    */

    DF_ADInit(&DF_source_ad, 4, 1);
    DF_ADInit(&DF_source_ad_b, 4, 1);
    DF_ADInit(&DF_output_ad, 4, 1);

    DF_FNInit1(&SOURCE_A_FN, &SOURCEA, "SA", 0);
    DF_FNInit2(&SOURCE_A_FN, 1, &DF_source_ad);

    DF_FNInit1(&SOURCE_B_FN, &SOURCEB, "SB", 0);
    DF_FNInit2(&SOURCE_B_FN, 1, &DF_source_ad_b);

    DF_FNInit1(&S_FN, &FUNS, "FS", 2, &DF_source_ad, &DF_source_ad_b);
    DF_FNInit2(&S_FN, 1, &DF_output_ad);

    DF_SourceInit(&DF_TFL_TABLE, 2, &SOURCE_A_FN, &SOURCE_B_FN);
    DF_Init(&DF_TFL_TABLE, 3, &SOURCE_A_FN, &SOURCE_B_FN, &S_FN);


    DF_Run(&DF_TFL_TABLE); // run and destory data
    int *b = DF_Result();
    
    // source tail
    printf("%d", *b);
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

