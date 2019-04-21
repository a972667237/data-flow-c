#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "threadpool.h"
#include "dfc.h"

DF_TFL DF_TFL_TABLE;

DF_AD DF_source_ad_a, DF_source_ad_b, DF_output_ad;

DF_FN SOURCE_A_FN, SOURCE_B_FN, Swap_FN;

int count;
int* array;

void SOURCEA() {
    int DF_count;
    int DF_source_A;

    DF_SOURCE_Get_And_Update(&SOURCE_A_FN, &DF_count);

    {
        DF_source_A = (2 * DF_count) % count;

        if (DF_count == (count * (count - 1)) ) {
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
        DF_source_B = ((2 * DF_count) + 1) % count;
        printf("%d\n", DF_source_B);   
        if (DF_count == (count * (count - 1))) {
            DF_Source_Stop(&DF_TFL_TABLE, 1);
        }
        
    }

    DF_AD_UpData(&DF_TFL_TABLE, &SOURCE_B_FN, &DF_source_B, sizeof(DF_source_B));
}


void swap() {
    int DF_source_A;
    int DF_source_B;
    int DF_output_A;

    DF_AD_GetData(&Swap_FN, &DF_source_A, sizeof(DF_source_A), &DF_source_B, sizeof(DF_source_B));

    {
        // fun swap
        if (array[DF_source_A] > array[DF_source_B]) {
            array[DF_source_A] = array[DF_source_A] ^ array[DF_source_B];
            array[DF_source_B] = array[DF_source_A] ^ array[DF_source_B];
            array[DF_source_A] = array[DF_source_A] ^ array[DF_source_B];
        }
    }

    DF_AD_UpData(&DF_TFL_TABLE, &Swap_FN, &DF_output_A, sizeof(DF_output_A));

}

int main() {
 //   system("clear");
    DF_ADInit(&DF_source_ad_a, 4, 1);
    DF_ADInit(&DF_source_ad_b, 4, 1);
    DF_ADInit(&DF_output_ad, 4, 0);

    DF_FNInit1(&SOURCE_A_FN, &SOURCEA, "SA", 0);
    DF_FNInit2(&SOURCE_A_FN, 1, &DF_source_ad_a);

    DF_FNInit1(&SOURCE_B_FN, &SOURCEB, "SB", 0);
    DF_FNInit2(&SOURCE_B_FN, 1, &DF_source_ad_b);

    DF_FNInit1(&Swap_FN, &swap, "FS", 2, &DF_source_ad_a, &DF_source_ad_b);
    DF_FNInit2(&Swap_FN, 1, &DF_output_ad);

    DF_SourceInit(&DF_TFL_TABLE, 2, &SOURCE_A_FN, &SOURCE_B_FN);
    DF_OutputInit(&DF_TFL_TABLE, 1, &DF_output_ad);
    DF_Init(&DF_TFL_TABLE, 3, &SOURCE_A_FN, &SOURCE_B_FN, &Swap_FN);
    
    count = 10;
    array = (int*)malloc(sizeof(int) * count);
    for (int i=0; i<count; i++) {
        array[count - i - 1] = i;
    }

    DF_Run(&DF_TFL_TABLE); // run and destory data

    for (int i=0; i<count; i++) {
        printf("%d ", array[i]);
    }
    return 0;
}
