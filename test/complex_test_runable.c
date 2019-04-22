#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "threadpool.h"
#include "dfc.h"

DF_TFL DF_TFL_TABLE;

DF_AD DF_source_ad_a, DF_source_ad_b, ad_c, ad_d, ad_e, ad_f, ad_g, ad_h, DF_output_ad_a, DF_output_ad_b;

DF_FN SOURCE_A_FN, SOURCE_B_FN, C_FN, D_FN, E_FN, F_FN, G_FN, H_FN;

void SOURCEA() {
    int DF_count;
    int DF_source_A;

    DF_SOURCE_Get_And_Update(&SOURCE_A_FN, &DF_count);

    {
        DF_source_A = DF_count;

        if (DF_count == 100) {
            DF_Source_Stop(&DF_TFL_TABLE, 0);
        }
        sleep(8);
    }

    DF_AD_UpData(&DF_TFL_TABLE, &SOURCE_A_FN, &DF_source_A, sizeof(DF_source_A));
}

void SOURCEB() {
    int DF_count;
    int DF_source_B;

    DF_SOURCE_Get_And_Update(&SOURCE_B_FN, &DF_count);

    {
        DF_source_B = DF_count * 2;

        if (DF_count == 100) {
            DF_Source_Stop(&DF_TFL_TABLE, 1);
        }
        sleep(2);
    }

    DF_AD_UpData(&DF_TFL_TABLE, &SOURCE_B_FN, &DF_source_B, sizeof(DF_source_B));
}

void FUNC() {
    int DF_source_A;
    int DF_source_B;
    int c;
    int d;
    int e;

    DF_AD_GetData(&C_FN, &DF_source_A, sizeof(DF_source_A), &DF_source_B, sizeof(DF_source_B));

    {
        c = DF_source_A + DF_source_B;
        d = DF_source_A - DF_source_B;
        e = DF_source_B * DF_source_A;
        sleep(4);
    }

    DF_AD_UpData(&DF_TFL_TABLE, &C_FN, &c, sizeof(c), &d, sizeof(d), &e, sizeof(e));
}

void FUND() {
    int c;
    int f;

    DF_AD_GetData(&D_FN, &c, sizeof(c));

    {
        f = c / 3;
        sleep(7);
    }

    DF_AD_UpData(&DF_TFL_TABLE, &D_FN, &f, sizeof(f));
}

void FUNE() {
    int d;
    int g;

    DF_AD_GetData(&E_FN, &d, sizeof(d));

    {
        g = -d;
        sleep(3);
    }

    DF_AD_UpData(&DF_TFL_TABLE, &E_FN, &g, sizeof(g));
}

void FUNF() {
    int e;
    int DF_output_A;

    DF_AD_GetData(&F_FN, &e, sizeof(e));

    {
        DF_output_A = e;
        sleep(1);
    }
    
    DF_AD_UpData(&DF_TFL_TABLE, &F_FN, &DF_output_A, sizeof(DF_output_A));
}

void FUNG() {
    int f;
    int h;

    DF_AD_GetData(&G_FN, &f, sizeof(f));
    {
        h = f;
        sleep(6);
    }
    DF_AD_UpData(&DF_TFL_TABLE, &G_FN, &h, sizeof(h));
}

void FUNH() {
    int g;
    int h;
    int DF_output_B;

    DF_AD_GetData(&H_FN, &g, sizeof(g), &h, sizeof(h));

    {
        DF_output_B = g + h;
        sleep(5);
    }

    DF_AD_UpData(&DF_TFL_TABLE, &H_FN, &DF_output_B, sizeof(DF_output_B));
}

int main() {
    system("clear");
    DF_ADInit(&DF_source_ad_a, 4, 1);
    DF_ADInit(&DF_source_ad_b, 4, 1);
    DF_ADInit(&ad_c, 4, 1);
    DF_ADInit(&ad_d, 4, 1);
    DF_ADInit(&ad_e, 4, 1);
    DF_ADInit(&ad_f, 4, 1);
    DF_ADInit(&ad_g, 4, 1);
    DF_ADInit(&ad_h, 4, 1);
    DF_ADInit(&DF_output_ad_a, 4, 0);
    DF_ADInit(&DF_output_ad_b, 4, 0);

    DF_FNInit1(&SOURCE_A_FN, &SOURCEA, "S_A", 0);
    DF_FNInit2(&SOURCE_A_FN, 1, &DF_source_ad_a);

    DF_FNInit1(&SOURCE_B_FN, &SOURCEB, "S_B", 0);
    DF_FNInit2(&SOURCE_B_FN, 1, &DF_source_ad_b);

    DF_FNInit1(&C_FN, &FUNC, "F_C", 2, &DF_source_ad_a, &DF_source_ad_b);
    DF_FNInit2(&C_FN, 3, &ad_c, &ad_d, &ad_e);

    DF_FNInit1(&D_FN, &FUND, "F_D", 1, &ad_c);
    DF_FNInit2(&D_FN, 1, &ad_f);

    DF_FNInit1(&E_FN, &FUNE, "F_E", 1, &ad_d);
    DF_FNInit2(&E_FN, 1, &ad_g);

    DF_FNInit1(&F_FN, &FUNF, "F_F", 1, &ad_e);
    DF_FNInit2(&F_FN, 1, &DF_output_ad_a);

    DF_FNInit1(&G_FN, &FUNG, "F_G", 1, &ad_f);
    DF_FNInit2(&G_FN, 1, &ad_h);

    DF_FNInit1(&H_FN, &FUNH, "F_H", 2, &ad_h, &ad_g);
    DF_FNInit2(&H_FN, 1, &DF_output_ad_b);

    DF_SourceInit(&DF_TFL_TABLE, 2, &SOURCE_A_FN, &SOURCE_B_FN);
    DF_OutputInit(&DF_TFL_TABLE, 2, &DF_output_ad_a, &DF_output_ad_b);

    DF_Init(&DF_TFL_TABLE, 8, &SOURCE_A_FN, &SOURCE_B_FN, &C_FN, &D_FN, &E_FN, &F_FN, &G_FN, &H_FN);

    DF_Run(&DF_TFL_TABLE);
    return 0;
}
