#include <stdio.h>
#include <stdlib.h>

void SOURCEA(;int DF_Source_A) {
    DF_Source_A = DF_count;
    sleep(4);
    if (DF_count == 9) {
        DF_Source_Stop();
    }
}

void SOURCEB(;int DF_Source_B) {
    DF_source_B = 100 - DF_count;
    sleep(1);
    if (DF_count == 9) {
        DF_Source_Stop();
    }
}

void FUNS(int DF_source_A, int DF_source_B; int DF_output_A) {
    DF_output_A = DF_source_A + DF_source_B;
    sleep(14);
    printf("total: -- %d\\n", DF_outout_A);
}

int main() {
    DF_Run();
    void** result = DF_Result();
    int* output = (int*)result[0];
    printf("result[0][0]: %d\\n", output[0]);
    return 0;
}

