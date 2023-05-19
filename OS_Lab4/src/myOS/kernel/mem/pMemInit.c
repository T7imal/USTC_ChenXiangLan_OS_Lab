#include "../../include/myPrintk.h"
#include "../../include/mem.h"
unsigned long pMemStart;  // 可用的内存的起始地址
unsigned long pMemSize;  // 可用的大小

void memTest(unsigned long start, unsigned long grainSize) {
	// TODO
	/*功能：检测算法
		这一个函数对应实验讲解ppt中的第一大功能-内存检测。
		本函数的功能是检测从start开始有多大的内存可用，具体算法参照ppt检测算法的文字描述
	注意点三个：
	1、覆盖写入和读出就是往指针指向的位置写和读，不要想复杂。
	  (至于为什么这种检测内存的方法可行大家要自己想一下)
	2、开始的地址要大于1M，需要做一个if判断。
	3、grainsize不能太小，也要做一个if判断

	*/
	if (start < 0x100000) {
		myPrintk(0x7, "Start address can't be less than 1M\n");
		return;
	}
	if (grainSize < 4) {
		myPrintk(0x7, "Grain size can't be less than 4 bytes\n");
		return;
	}
	unsigned char* p = start;
	unsigned char ch1, ch2, ch3, ch4;
	pMemSize = 0;
	while (1) {
		ch3 = *p;
		ch4 = *(p + 1);
		*p = 0xAA;
		*(p + 1) = 0x55;
		ch1 = *p;
		ch2 = *(p + 1);
		if (ch1 != 0xAA || ch2 != 0x55) break;
		*p = 0x55;
		*(p + 1) = 0xAA;
		ch1 = *p;
		ch2 = *(p + 1);
		if (ch1 != 0x55 || ch2 != 0xAA) break;
		*p = ch3;
		*(p + 1) = ch4;	//写回原值
		ch3 = *(p + grainSize - 1);
		ch4 = *(p + grainSize - 2);
		*(p + grainSize - 1) = 0xAA;
		*(p + grainSize - 2) = 0x55;
		ch1 = *(p + grainSize - 1);
		ch2 = *(p + grainSize - 2);
		if (ch1 != 0xAA || ch2 != 0x55) break;
		*(p + grainSize - 1) = 0x55;
		*(p + grainSize - 2) = 0xAA;
		ch1 = *(p + grainSize - 1);
		ch2 = *(p + grainSize - 2);
		if (ch1 != 0x55 || ch2 != 0xAA) break;
		*(p + grainSize - 1) = ch3;
		*(p + grainSize - 2) = ch4;	//写回原值
		p += grainSize;
		pMemSize += grainSize;
	}
	if (p == start) {
		myPrintk(0x7, "No available memory\n");
		return;
	}
	pMemStart = start;
	myPrintk(0x7, "MemStart: %x  \n", pMemStart);
	myPrintk(0x7, "MemSize:  %x  \n", pMemSize);

}

extern unsigned long _end;
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
	pMemHandler = dPartitionInit(pMemStart, pMemSize);
}
