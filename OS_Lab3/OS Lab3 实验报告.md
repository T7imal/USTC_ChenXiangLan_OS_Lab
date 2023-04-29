# OS Lab3 实验报告

### 实验目标

1. 实现简单的`shell`程序，提供`cmd`和`help`命令，允许注册新的命令 

2. 实现中断机制和中断控制器`i8259A`初始化 

3. 实现时钟`i8253`和周期性时钟中断 

4. 实现`VGA`输出的调整： 

   – 时钟中断之外的其他中断，一律输出`Unknown interrupt`。 

   – 右下角：从某个时间开始，大约每秒更新一次，格式为：`HH:MM:SS` 。

### 实验原理

1. 本次软件启动的过程大部分与上一次实验相同，除此之外，软件加入了中断向量表的建立、初始化，并写入了时间中断（转到`time_interrupt`，调用`tick()`函数）和其他事件中断（转到`ignore_int1`，调用`ignoreIntBody()`函数）
1. 接下来程序转到`osStart.c`文件中，调用`osStart()`函数。该函数首先配置可编程逻辑芯片`i8253`和`i8259A`，建立时钟，并调用`tick()`函数，使初始时间显示出来。然后调用`enable_interrupt`，启动中断
1. 接下来程序清屏进入`myMain()`函数，调用`startShell()`函数，从串口中读取输入指令，并识别指令并执行（`cmd`，`help`）

### 源代码说明

**`myOS/start32.S` 中的`time_interrupt`和`ignore_int1`的填写**

均为：清零方向标志位；将寄存器内容入栈；调用函数；将寄存器内容出栈；返回

```assembly
time_interrupt:
//你需要填写它
    cld
    pushf
    pusha
    call tick
    popa
    popf
    iret

ignore_int1:
//你需要填写它
	cld
	pushf
    pusha
    call ignoreIntBody
    popa
    popf
    iret
```

**`myOS/dev/i8253.c`和`myOS/dev/i8259A.c`的填写**

```c
void init8253(void){
	//你需要填写它
	outb(0x43, 0x34);
	outb(0x40, 0x9c);
	outb(0x40, 0x2e);
}
void init8259A(void){
	//你需要填写它
	outb(0x21, 0xFF);
	outb(0xA1, 0xFF);
	outb(0x20, 0x11);
	outb(0x21, 0x20);
	outb(0x21, 0x04);
	outb(0x21, 0x03);
	outb(0xA0, 0x11);
	outb(0xA1, 0x28);
	outb(0xA1, 0x02);
	outb(0xA1, 0x01);
}
```

**`myOS/i386/irq.s`的填写**

```assembly
.text
.code32
_start:
	.globl enable_interrupt
enable_interrupt:
//你需要填写它
    sti	#开中断
    ret
	.globl disable_interrupt
disable_interrupt:
//你需要填写它
    cli	#关中断
    ret
```

**`myOS/kernel/wallclock.c`的填写**

```c
void setWallClock(int HH,int MM,int SS){
	//你需要填写它
	char time[8];
	char* vga_p=(char*) 0xB8F90;//写入到VGA显存最后一行的最后
	time[0]=(char)(HH/10)+48;
	time[1]=(char)(HH%10)+48;
	time[2]=':';
	time[3]=(char)(MM/10)+48;
	time[4]=(char)(MM%10)+48;
	time[5]=':';
	time[6]=(char)(SS/10)+48;
	time[7]=(char)(SS%10)+48;
	for(int i=0;i<8;++i){
	    *vga_p++=time[i];
	    *vga_p++=0x20;
	}
}
void getWallClock(int *HH,int *MM,int *SS){
	//你需要填写它
	char time[10];
	char* vga_p=(char*) 0xB8F90;
	*HH=(*vga_p-48)*10+(*(vga_p+2)-48);
	vga_p+=6;
	*MM=(*vga_p-48)*10+(*(vga_p+2)-48);
	vga_p+=6;
	*SS=(*vga_p-48)*10+(*(vga_p+2)-48);
}
```

**`myOS/kernel/tick.c`的填写**

```c
#include "wallClock.h"
int system_ticks;
int HH=0,MM=0,SS=0;//初始化时间为0
void tick(void){
	system_ticks++;//可编程逻辑芯片的中断频率为100HZ
	//你需要填写它
	if(system_ticks==100){//每秒一次
	    system_ticks=0;
	    if(++SS==60){
	        SS=0;
	        if(++MM==60){
	            MM=0;
	            ++HH;
	        }
	    }
	}
	setWallClock(HH,MM,SS);
	return;
}
```

`userApp/startShell.c`的填写

```c
int func_cmd(int argc, char (*argv)[8]){
	myPrintk(0x02, "cmd\n\0");
	myPrintk(0x02, "help\n\0");//打印指令列表
}
int strcmp(char*a, char*b){
    while(*a&&*b){
        if(*a!=*b) return 0;
        ++a;++b;
    }
    if(!*a&&!*b) return 1;
    return 0;
}//若字符串相同则返回1，若不同则返回0
int func_help(int argc, char (*argv)[8]){
    if(argc==2){
        if(strcmp(argv[1], "cmd\0"))
            myPrintk(0x02, cmd.help_content);//除输入help cmd之外，都输出help指令的帮助内容
        else
            myPrintk(0x02, help.help_content);
    }
    else
        myPrintk(0x02, help.help_content);
}
void startShell(void){
//我们通过串口来实现数据的输入
char BUF[256]; //输入缓存区
int BUF_len=0;	//输入缓存区的长度
    
	int argc;
    char argv[8][8];

    do{
        BUF_len=0; 
        myPrintk(0x07,"Student>>\0");
        while((BUF[BUF_len]=uart_get_char())!='\r'){
            uart_put_char(BUF[BUF_len]);//将串口输入的数存入BUF数组中
            BUF_len++;  //BUF数组的长度加
        }
        uart_put_chars(" -pseudo_terminal\0");
        uart_put_char('\n');

        //OK,助教已经帮助你们实现了“从串口中读取数据存储到BUF数组中”的任务，接下来你们要做
        //的就是对BUF数组中存储的数据进行处理(也即，从BUF数组中提取相应的argc和argv参
        //数)，再根据argc和argv，寻找相应的myCommand ***实例，进行***.func(argc,argv)函数
        //调用。

        //比如BUF中的内容为 “help cmd”
        //那么此时的argc为2 argv[0]为help argv[1]为cmd
        //接下来就是 help.func(argc, argv)进行函数调用即可
        argc=1;int j=0;
        for(int i=0;i<BUF_len;++i){
            if(BUF[i]==' '){
                argv[argc++-1][j]='\0';
                j=0;
            }//若遇到空格，则++argc，将BUF的下一部分填入argv的下一部分中
            else{
                argv[argc-1][j++]=BUF[i];
            }//将BUF中不是空格的内容填入argv中
        }
        argv[argc-1][j]='\0';
        if(strcmp(argv[0], "cmd\0"))
            cmd.func(argc, argv);
        else if(strcmp(argv[0], "help\0"))
            help.func(argc, argv);
        else
            myPrintk(0x02, "The cmd is not defined\n\0");
    }while(1);
}
```

### 代码布局说明

`multiboot_header`的起始地址为`1M`，`.text`部分起始地址为其后补足至8字节对齐

`.data`部分为其后补足至16字节对齐

`.bss`部分为其后补足至16字节对齐

`.end`部分为其后补足至16字节对齐

后面补足至512字节对齐

### 编译过程说明

输入`./source2run.sh`指令后，编译，链接，生成`myOS.elf`文件（明明同时就运行了）。输入指令`qemu-system-i386 - kernel myOS.elf -serial pty &`运行。串口重定向到伪终端`/dev/pts/1`，输入`sudo screen /dev/pts/1`进入交互界面，即可执行`cmd`，`help`命令

### 实验结果

![image-20230429152952890](C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230429152952890.png)

### 遇到的问题和解决方法

对比字符串——自己完成字符对比函数