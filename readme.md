## Data-Flow-C
---
DF-C 是一个数据流编程语言，它提供了对`C语言`语法的扩展，旨在简化多线程编程

### Examples
```c
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
```

### 语法说明
#### 对函数的扩展
```c
void SOURCEA(;void* c)
// 源主动函数
// 由于分号前没有数据定义，故而为无输入参数，为源主动函数
// 定义输出主动数据 void* c


void B(void* c;int x, int y ,int** z)
// 普通函数
// 分号前是输入参数，取 SOURCEA 的输出主动数据，然后再输出三个数据
```
其定义的源主动函数将由主线程统一调度生产
普通函数由回调触法运行
源主动函数还提供
```c
DF_count; // 当前方法使用次数
DF_source_stop; // 当前方法停止接受输入流
```

#### 调用 DF-C 的方法
```c
DF_Thread_init(void* pool, int threadnum, int queuesize); 
// 若是不想通过编译指令指定线程数，则可以直接在主线程调用此函数进行指定的初始化

DF_Run();
// 开始运行 DF-C

DF_Result();
// 获取 DF-C 运行的最终结果，返回的数据结果是 void**
```

#### 宏调用
我们定义了两个宏，包括是否 DEBUG 和线程数
```makefile
make DEBUG=True // 打开内容打印
make THREADNUM=16 // 指定线程数
```
