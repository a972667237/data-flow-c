#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "threadpool.h"
#include "dfc.h"

DF_TFL DF_TFL_TABLE;

DF_AD DF_source_ad_a;

DF_FN SOURCE_A_FN;

int width_A, width_B, height_B;
int **A, **B, **C;

void SOURCEA() {
    int DF_count;
    int DF_source_A;

    DF_SOURCE_Get_And_Update(&SOURCE_A_FN, &DF_count);

    {
        if (DF_count == width_A * height_B)
            DF_Source_Stop(&DF_TFL_TABLE, 0);
        int i = DF_count / height_B;
        int j = DF_count % height_B;
        if (i>=width_A)
            return;
        C[i][j] = 0;
        for (int k=0; k<width_B; k++)
            C[i][j] += A[i][k] * B[j][k];
    }
}

int main() {
    DF_ADInit(&DF_source_ad_a, 4, 0);

    DF_FNInit1(&SOURCE_A_FN, &SOURCEA, "SA", 0);
    DF_FNInit2(&SOURCE_A_FN, 1, &DF_source_ad_a);

    DF_SourceInit(&DF_TFL_TABLE, 1, &SOURCE_A_FN);
    DF_OutputInit(&DF_TFL_TABLE, 1, &SOURCE_A_FN);
    DF_Init(&DF_TFL_TABLE, 1, &SOURCE_A_FN);

    width_A = 10000;
    width_B = 10000;
    height_B = 10000;
    
    A = (int**)malloc(sizeof(int*) * width_A);
    B = (int**)malloc(sizeof(int*) * width_B);
    C = (int**)malloc(sizeof(int*) * width_A);

    for (int i=0; i<width_A; i++) {
        A[i] = (int*)malloc(sizeof(int) * width_B);
        for (int j=0; j<width_B; j++) {
            A[i][j] = -1;
        }
    }
    for (int i=0; i<width_B; i++) {
        B[i] = (int*)malloc(sizeof(int) * height_B);
        for (int j=0; j<height_B; j++) {
            B[i][j] = 1;
        }
    }
    for (int i=0; i<width_A; i++) {
        C[i] = (int*)malloc(sizeof(int) * height_B);
    }

    DF_Run(&DF_TFL_TABLE);
    return 0;
}
