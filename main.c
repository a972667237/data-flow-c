#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "threadpool.h"
#include "dfc.h"


DF_TFL DF_TFL_TABLE;

DF_AD DF_source_ad_a, DF_source_ad_b, DF_output_ad;

DF_FN SOURCE_A_FN, SOURCE_B_FN, S_FN;

void SOURCEA() {
    int DF_count;
    int DF_source_A;

    DF_SOURCE_Get_And_Update(&SOURCE_A_FN, &DF_count);

    {
        DF_source_A = DF_count;
        sleep(4);

        if(DF_count == 99) {
            DF_Source_Stop(&DF_TFL_TABLE, 0);
        }
    }
    DF_AD_UpData(&DF_TFL_TABLE, &SOURCE_A_FN, &DF_source_A, sizeof(DF_source_A));
}

void SOURCEB() {
    int DF_count;
    int DF_source_B;

    DF_SOURCE_Get_And_Update(&SOURCE_B_FN, &DF_count);

    {
        DF_source_B = 100 - DF_count;
        sleep(1);
//        printf("b: %d\n", DF_source_B);

        if (DF_count == 99) {
            DF_Source_Stop(&DF_TFL_TABLE, 1);
        }
    }
    DF_AD_UpData(&DF_TFL_TABLE, &SOURCE_B_FN, &DF_source_B, sizeof(DF_source_B));
}

void FUNS() {
    int DF_source_A;
    int DF_source_B;
    int DF_output_A;

    DF_AD_GetData(&S_FN, &DF_source_A, sizeof(DF_source_A), &DF_source_B, sizeof(DF_source_B));

    {
        // fun S
        DF_output_A = DF_source_A + DF_source_B;
        //printf("output: %d\n", DF_output_A);
        sleep(14);
        //system("clear");
        //printf("total: -- %d\n", DF_output_A);
    }
    DF_AD_UpData(&DF_TFL_TABLE, &S_FN, &DF_output_A, sizeof(DF_output_A));
}

int main() {
    system("clear");
    DF_ADInit(&DF_source_ad_a, 4, 1);
    DF_ADInit(&DF_source_ad_b, 4, 1);
    DF_ADInit(&DF_output_ad, 4, 0);

    DF_FNInit1(&SOURCE_A_FN, &SOURCEA, "SA", 0);
    DF_FNInit2(&SOURCE_A_FN, 1, &DF_source_ad_a);

    DF_FNInit1(&SOURCE_B_FN, &SOURCEB, "SB", 0);
    DF_FNInit2(&SOURCE_B_FN, 1, &DF_source_ad_b);

    DF_FNInit1(&S_FN, &FUNS, "FS", 2, &DF_source_ad_a, &DF_source_ad_b);
    DF_FNInit2(&S_FN, 1, &DF_output_ad);

    DF_SourceInit(&DF_TFL_TABLE, 2, &SOURCE_A_FN, &SOURCE_B_FN);
    DF_Init(&DF_TFL_TABLE, 3, &SOURCE_A_FN, &SOURCE_B_FN, &S_FN);


    DF_Run(&DF_TFL_TABLE); // run and destory data
    
    return 0;
}
