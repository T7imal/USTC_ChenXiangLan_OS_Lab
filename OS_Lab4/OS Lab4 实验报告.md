# OS Lab4 实验报告

### 实验目标

1. 在Lab3的基础上，为OS内核实现内存管理功能，为用户提供`malloc/free`接口，为内核提供`kmalloc/kfree`接口，

### 源代码说明

**在我的实现中，所有的结构体中的`size`项指的都是包含结构体本身的内存大小**

动态内存管理中，空闲链表的初始化：

```c
unsigned long dPartitionInit(unsigned long start, unsigned long totalSize) {
	dPartition* dp = start;
	dp->firstFreeStart = start + dPartition_size;
	dp->size = totalSize;
	EMB* emb = dp->firstFreeStart;
	emb->size = totalSize - dPartition_size;
	emb->nextStart = 0x0;	//最后一个空闲块的nextStart为0x0
	emb->userData = 0x0;
	return start;
}
```

动态内存分配：

```c
unsigned long dPartitionAllocFirstFit(unsigned long dp, unsigned long size) {
	dPartition* dpp = dp;
	if (size <= 0) {
		return 0;
	}
	EMB* emb = dpp->firstFreeStart;
	if (emb->size >= size + EMB_size) {	//第一个空闲块的体积大于分配需要的内存，将其去出链表
		EMB* new_emb = (unsigned long)emb + size + EMB_size;
		new_emb->nextStart = emb->nextStart;
		new_emb->size = emb->size - size - EMB_size;
		emb->size = size + EMB_size;	//保留句柄，方便释放
		dpp->firstFreeStart = new_emb;
		return (unsigned long)emb + EMB_size;
	}
	while (emb->nextStart) {	//寻找第一个体积大于分配需要的内存的空闲块，将其去出链表
		EMB* nextEmb = emb->nextStart;
		if (nextEmb->size >= size + EMB_size) {
			break;
		}
		emb = emb->nextStart;
	}
	EMB* nextEmb = emb->nextStart;
	if (nextEmb == 0x0) {
		return 0;
	}
	EMB* new_emb = (unsigned long)nextEmb + size + EMB_size;
	new_emb->nextStart = nextEmb->nextStart;
	new_emb->size = nextEmb->size - size - EMB_size;
	nextEmb->size = size + EMB_size;	//保留句柄，方便释放
	emb->nextStart = new_emb;
	return (unsigned long)nextEmb + EMB_size;
}
```

动态内存释放：

```c
unsigned long dPartitionFreeFirstFit(unsigned long dp, unsigned long start) {
	dPartition* dpp = dp;
	EMB* bp = start;
	if (start < dp || start - EMB_size + ((EMB*)(start - EMB_size))->size > dp + dpp->size) {	//检查要释放的范围是否在dp有效范围内
		return 0;
	}
	EMB* emb = dpp->firstFreeStart;
	if (emb >= bp) {
		dpp->firstFreeStart = start - EMB_size;
		((EMB*)(start - EMB_size))->nextStart = emb;
		if (start - EMB_size + ((EMB*)(start - EMB_size))->size == emb) {	//左侧合并
			((EMB*)(start - EMB_size))->size += emb->size;
			((EMB*)(start - EMB_size))->nextStart = emb->nextStart;
		}
		return 1;
	}
	while (emb->nextStart) {
		if (emb->nextStart >= bp) {
			break;
		}
		emb = emb->nextStart;
	}
	EMB* nextEmb = emb->nextStart;
	emb->nextStart = start - EMB_size;
	((EMB*)(start - EMB_size))->nextStart = nextEmb;
	if ((unsigned long)emb + emb->size == start - EMB_size && nextEmb == start - EMB_size + ((EMB*)(start - EMB_size))->size) {	//左右都合并
		emb->size += ((EMB*)(start - EMB_size))->size + nextEmb->size;
		emb->nextStart = nextEmb->nextStart;
	}
	else if ((unsigned long)emb + emb->size == start - EMB_size) {	//左侧合并
		emb->size += ((EMB*)(start - EMB_size))->size;
		emb->nextStart = nextEmb;
	}
	else if (nextEmb == start - EMB_size + ((EMB*)(start - EMB_size))->size) {	//右侧合并
		((EMB*)(start - EMB_size))->size += nextEmb->size;
		((EMB*)(start - EMB_size))->nextStart = nextEmb->nextStart;
	}
	return 1;
}
```

等大小内存管理中，空闲链表的实际大小计算：

```c
unsigned long eFPartitionTotalSize(unsigned long perSize, unsigned long n) {
	if ((perSize + EEB_size) % 8 == 0)	//8字节对齐
		return (perSize + EEB_size) * n + eFPartition_size;
	else
		return ((perSize + EEB_size) / 8 + 1) * 8 * n + eFPartition_size;
}
```

等大小内存管理中，空闲链表的初始化：

```c
unsigned long eFPartitionInit(unsigned long start, unsigned long perSize, unsigned long n) {
	eFPartition* ef = start;
	ef->totalN = n;
	if ((perSize + EEB_size) % 8 == 0)
		ef->perSize = perSize + EEB_size;
	else
		ef->perSize = ((perSize + EEB_size) / 8 + 1) * 8;
	EEB* eeb = start + eFPartition_size;
	ef->firstFree = eeb;
	for (unsigned long i = 0;i < n - 1;++i) {	//生成长度为n的空闲链表
		eeb->next_start = (unsigned long)eeb + ef->perSize;
		eeb = eeb->next_start;
	}
	eeb->next_start = 0x0;
	return ef;
```

等大小内存分配：

```c
unsigned long eFPartitionAlloc(unsigned long EFPHandler) {
	eFPartition* ef = EFPHandler;
	EEB* eeb = ef->firstFree;
	if (!eeb) {
		return 0;
	}
	if (eeb) {
		ef->firstFree = eeb->next_start;
	}
	return (unsigned long)eeb + EEB_size;
}
```

等大小内存释放：

```c
unsigned long eFPartitionFree(unsigned long EFPHandler, unsigned long mbStart) {
	eFPartition* ef = EFPHandler;
	EEB* eeb = ef->firstFree;
	if (mbStart < EFPHandler || mbStart - eFPartition_size + ef->perSize > EFPHandler + eFPartition_size + ef->totalN * ef->perSize) {	//检查要释放的范围是否在有效范围内
		return 0;
	}
	if (!ef->firstFree || eeb >= mbStart) {	//释放的内存块在第一个空闲块之前
		EEB* newEeb = mbStart - EEB_size;
		newEeb->next_start = eeb;
		ef->firstFree = newEeb;	//插入释放的空闲块
		return 1;
	}
	while (eeb->next_start) {	//找到释放的内存块左右的空闲块
		if (eeb->next_start >= mbStart) {
			break;
		}
		eeb = eeb->next_start;
	}
	EEB* newEeb = mbStart - EEB_size;
	newEeb->next_start = eeb->next_start;
	eeb->next_start = newEeb;	//插入释放的空闲块
	return 1;
}
```

`kmalloc/kfree`的实现：

（在用户内存之前，额外空出1M内存作为内核内存管理）

```c
unsigned long kpMemHandler;

void pMemInit(void) {
	unsigned long _end_addr = (unsigned long)&_end;
	memTest(0x100000, 0x1000);
	myPrintk(0x7, "_end:  %x  \n", _end_addr);
	if (pMemStart <= _end_addr) {
		pMemSize -= _end_addr - pMemStart;
		pMemStart = _end_addr;
		myPrintk(0x7, "MemStart: %x  \n", pMemStart);
		myPrintk(0x7, "MemSize:  %x  \n", pMemSize);
	}

	// 此处选择不同的内存管理算法
	kpMemHandler = dPartitionInit(pMemStart, 0x100000);	//内核内存管理
	pMemHandler = dPartitionInit(pMemStart + 0x100000, pMemSize - 0x100000);	//用户内存管理
}

unsigned long kmalloc(unsigned long size) {
    // 调用实现的dPartition或者是ePartition的alloc
    return dPartitionAlloc(kpMemHandler, size);

}

unsigned long kfree(unsigned long start) {
    // 调用实现的dPartition或者是ePartition的free
    return dPartitionFree(kpMemHandler, start);
}
```

`kmalloc/kfree`的测试：

```c
int testKmalloc(int argc, unsigned char** argv) {
	//======for kmalloc===============================
	char* buf1 = (char*)kmalloc(11);
	char* buf2 = (char*)kmalloc(21);

	for (int i = 0;i < 9;i++) *(buf1 + i) = '+';
	*(buf1 + 9) = '\n';
	*(buf1 + 10) = '\000';

	for (int i = 0;i < 19;i++) *(buf2 + i) = ',';
	*(buf2 + 19) = '\n';
	*(buf2 + 20) = '\000';

	myPrintf(0x5, "We allocated 2 buffers.\n");
	myPrintf(0x5, "BUF1(size=9, addr=0x%x) filled with 9(+): ", (unsigned long)buf1);
	myPrintf(0x7, buf1);
	myPrintf(0x5, "BUF2(size=19, addr=0x%x) filled with 19(,): ", (unsigned long)buf2);
	myPrintf(0x7, buf2);

	myPrintf(0x7, "\n");

	kfree((unsigned long)buf1);
	kfree((unsigned long)buf2);

	return 0;
}
```

### 问题回答

**请写出动态分配算法的`malloc`接口是如何实现的（即`malloc`函数调用了哪个函数，这个函数又调用了哪个函数...）**

`malloc(size)`函数调用`dPartitionAlloc(pMemHandler, size)`，其中`pMemHandler`是`pMemInit()`中计算得出的用户内存初始地址，也即用户动态内存管理的结构体地址，接下来调用`dPartitionAllocFirstFit(pMemHandler, size)`，即在`pMemHandler`指向的动态内存管理结构体代表的内存中，分配一段大小为`size`的内存，并返回这段内存的地址

**运行`memTestCaseInit`那些新增的shell命令，会出现什么结果，即打印出什么信息（截图放到报告中）？是否符合你的预期，为什么会出现这样的结果。（详细地讲一两个运行结果，大同小异的可以从简）**

`malloc/free`和`kmalloc/kfree`的实现，其中`malloc/free`使用的空间为`kmalloc/kfree`使用的空间之后1M的空间：

<img src="C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230521154814140.png" alt="image-20230521154814140"  />

动态内存分配：

分配得到的地址`0x206260`为动态内存管理首地址`0x206250`加上`dPartition_size + EMB_size`，空闲块`size`为`0x100 - dPartition_size `（包含结构体体积）

![image-20230521155233873](C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230521155233873.png)

（QEMU VGA屏幕无法完全显示）

分配一个大小为`0x10`的内存，则生成的新的空闲块的位置为其后`EMB_size + 0x10`，即`0x206258 + EMB_size + 0x10 = 0x206270`，下同

当三块内存被全部释放时，空闲块合并

<img src="C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230521155346056.png" alt="image-20230521155346056"  />

与上同理：

（图中注释有误，A:B:C:- ==> A:B:- ==> A:- ==> -）

![image-20230521160300980](C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230521160300980.png)

等大小内存分配：

分配时设定的`perSize`为`31`，为了8字节对齐（包含空闲块结构体），因此初始化的结构体的`perSize`为`((31+4)/8+1)*8=40`，也即`0x28`，两个空闲块之间的距离为`0x28`

![image-20230521160625476](C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230521160625476.png)

### 遇到的问题和解决方法

地址计算错误——将指针变量强制转换为`unsigned long`变量