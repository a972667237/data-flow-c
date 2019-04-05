#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include "threadpool.h"
#include "dfc.h"


DF_TFL DF_TFL_TABLE;

DF_AD DF_source_ad, DF_source_ad_b, DF_output_ad;

DF_FN SOURCE_A_FN, SOURCE_B_FN, S_FN;

void SOURCEA() {
    int DF_count;
    int DF_source;

    DF_SOURCE_Get_And_Update(&SOURCE_A_FN, &DF_count);

    {
        DF_source = DF_count;

        if(DF_count == 9) {
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

        if (DF_count == 9) {
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

int main() {
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
    // compile


    DF_Run(&DF_TFL_TABLE); // run and destory data
    
    // source tail
    return 0;
    // source tail end
}
