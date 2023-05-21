#include "../../include/mem.h"

// 只对内核提供kmalloc/kfree接口


unsigned long kmalloc(unsigned long size) {
    // 调用实现的dPartition或者是ePartition的alloc
    return dPartitionAlloc(kpMemHandler, size);

}

unsigned long kfree(unsigned long start) {
    // 调用实现的dPartition或者是ePartition的free
    return dPartitionFree(kpMemHandler, start);
}