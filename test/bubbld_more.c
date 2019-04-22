#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "threadpool.h"
#include "dfc.h"

DF_TFL DF_TFL_TABLE;

DF_AD DF_source_ad_a;

DF_FN SOURCE_A_FN;

int count;
int* array;
int count_to_index;

void SOURCEA() {
    int DF_count;
    int DF_source_A;

    DF_SOURCE_Get_And_Update(&SOURCE_A_FN, &DF_count);

    {
        if (count % 2 == 0)
            DF_source_A = (2 * DF_count) % count_to_index;
        else
            DF_source_A = (2 * DF_count) % count_to_index + ((DF_count % count_to_index) >= count_to_index/2);

        if (array[DF_source_A] > array[DF_source_A + 1]) {
            array[DF_source_A] = array[DF_source_A] ^ array[DF_source_A + 1];
            array[DF_source_A + 1] = array[DF_source_A] ^ array[DF_source_A + 1];
            array[DF_source_A] = array[DF_source_A] ^ array[DF_source_A + 1];
        }

        if (DF_count == (count * count_to_index)/2 ) {
            DF_Source_Stop(&DF_TFL_TABLE, 0);
        }
    }

    DF_AD_UpData(&DF_TFL_TABLE, &SOURCE_A_FN, &DF_source_A, sizeof(DF_source_A));
}


int main() {
 //   system("clear");
    DF_ADInit(&DF_source_ad_a, 4, 0);

    DF_FNInit1(&SOURCE_A_FN, &SOURCEA, "SA", 0);
    DF_FNInit2(&SOURCE_A_FN, 1, &DF_source_ad_a);


    DF_SourceInit(&DF_TFL_TABLE, 1, &SOURCE_A_FN);
    DF_OutputInit(&DF_TFL_TABLE, 1, &SOURCE_A_FN);
    DF_Init(&DF_TFL_TABLE, 1, &SOURCE_A_FN);
    
    count = 10000;
    count_to_index = count - 1;
    array = (int*)malloc(sizeof(int) * count);
    for (int i=0; i<count; i++) {
        array[count - i - 1] = i;
    }

    DF_Run(&DF_TFL_TABLE); // run and destory data

    return 0;
}
