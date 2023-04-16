# OS Lab1 实验报告

### 实验目标

1. 使用VMware配置ubuntu虚拟机，并安装QEMU、GCC等
2. 填写`Makefile`文件、`multibootHeader.ld`文件，并理解其中的内容和作用
3. 编写`multibootHeader.s`文件，并编译生成`multibootHeader.bin`文件并运行，分别在VGA和串口输出内容

### 实验原理

1. Multiboot启动协议将系统设置为一个明确的状态，例如用户可以读写所有内存；操作系统内核被设置为ELF格式等
2. QEMU支持Multiboot协议，通过设定Multiboot Header参数，可以使用QEMU模拟内核
3. 编写汇编代码，将内容写入内核的VGA显存即可输出
4. 编写汇编代码，将内容传到UART串口地址即可输出
5. 使用指令`make`将源文件汇编为内核文件`multibootHeader.bin`
6. 使用指令` qemu-system-i386 -kernel multibootHeader.bin -serial stdio`指定平台为i386，指定内核文件为`multibootHeader.min`，并指定串行终端为标准输入输出，并启动QEMU

### 源代码说明

关键代码说明为第29-31行、45-46行

```assembly
.globl start #一般都用start

/*此处，按照multiboot协议要求定义必要的参数*/
/*格式：XXX_ITEM_NAME=value*/
/*value可以是十六进制的（0x开头）、十进制的等等*/
Lab1_magic=0x1BADB002
Lab1_flags=0
Lab1_checksum=0xE4524FFE

/*此处开始，按协议标准来定义必须的multiboot header*/
.section ".Lab1" #先起一个section的名字
.align 4 # 补充，需要4字节对齐，不然机器找不到启动头

/*使用.long和前面定义的参数构建头结构，每次32位，格式为：.long XXX_ITEM_NAME*/
.long Lab1_magic
.long Lab1_flags
.long Lab1_checksum

.text #进入代码段
.code32 #32位代码

start: #这个跟第一行的声明要一致

/*下面屏幕输出OK或其他字符序列*/
/*使用指令movl $0x12345678, 0xB8000*/ #0x1234和0x5678各自输出1个字符
/*根据需要输出多个字符，也可以使用其他mov指令*/
/*可以根据需要使用nop指令隔开不同功能片段，也可以适当使用空行*/
movl $0x2f572f48, 0xB8000	#HW
/*mvol指令将数据0x2f572f48传到内存地址0xB8000，其中2f代表绿底白字，57和48分别是W和H的ASCII码。*/
/*这个指令的意思是在VGA显存的首位输出HW字符*/
/*由于系统运行在小端模式，要输出HW字符，需要以WH的顺序输入*/
movl $0x2f5f2f43, 0xB8004	#C_
movl $0x2f422f50, 0xB8008	#PB
movl $0x2f312f32, 0xB800c	#21
movl $0x2f302f30, 0xB8010	#00
movl $0x2f322f30, 0xB8014	#02
movl $0x2f392f30, 0xB8018	#09

/*根据需要初始化串口*/
/*根据需要串口输出你的字符序列，详见前面串口编程简介*/
/*实验结束，让计算机停机，方法：使用hlt指令，或者死循环*/
movb $0x48, %al
movw $0x3F8, %dx
outb %al, %dx      #H
/*这段指令将要输出的字符的ASCII码储存在寄存器中，将串口的地址也储存在寄存器中*/
/*outb指令通过寄存器寻址将要输出的字符通过串口输出到标准输入输出中*/
movb $0x55, %al
outb %al, %dx      #U
movb $0x41, %al
outb %al, %dx      #A
movb $0x4e, %al
outb %al, %dx      #N
movb $0x47, %al
outb %al, %dx      #G
movb $0x57, %al
outb %al, %dx      #W
movb $0x41, %al
outb %al, %dx      #A
movb $0x4e, %al
outb %al, %dx      #N
movb $0x43, %al
outb %al, %dx      #C
movb $0x48, %al
outb %al, %dx      #H
movb $0x41, %al
outb %al, %dx      #A
movb $0x4f, %al
outb %al, %dx      #O
hlt
```

### 代码布局说明

`multibootHeader.s`中`multibootHeader`在内存中占12字节

需要在声明`section`后增加4字节对齐，使机器能够找到启动头

VGA显存是地址`0xB8000`及更高地址的内容，每两字节输出一个字符，这2字节分别是字符的样式和ASCII码

因为每个`movl`指令输出两个字符，占用4字节的显存，所以每两个`movl`指令中间目标地址相差为4

### 编译过程说明

输入`make`指令即可生成内核文件`multibootHeader.bin`

输入` qemu-system-i386 -kernel multibootHeader.bin -serial stdio`指令即可运行QEMU

### 实验结果

<img src="C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230327142047241.png" alt="image-20230327142047241"  />

### 遇到的问题和解决方法

VGA输出每两个字符的顺序相反——将`movl`指令中两个字符的ASCII码交换