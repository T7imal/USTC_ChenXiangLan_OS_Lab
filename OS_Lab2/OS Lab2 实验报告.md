# OS Lab2 实验报告

### 实验目标

1. 在软件层次和结构上，完成multiboot_header、myOS和userApp的划分，体现在文件目录组织和Makefile组织上，理解小型操作系统的文件结构
1. 实现清屏、格式化输入输出的功能，设备包括VGA和串口
1. 完成从汇编语言到C语言的转变

### 实验原理

1. 通过脚本编译并执行`void osStart(void)`函数，启动系统，并执行`void myMain(void)`函数，其中调用串口输出、VGA输出、格式化VGA输出等功能

2. 程序从`multibootHeader`开始执行，其中的代码部分直接调用`_start`标识的代码段（位于`start32.S`中）

   ```assembly
   .text
   .code32
   start:
     call _start
     hlt  
   ```

3. 接下来进入`establish_stack`段和`zero_bss`段，功能分别为分别建立C语言程序运行所需的栈空间（位于结构的最后），和清零bss段

4. 在`osStart.c`文件中，`void osStart(void)`调用在`myOS`、`userApp`文件夹中的各种功能函数，实现所需的功能

### 源代码说明

```assembly
# Set up the stack
establish_stack:
	movl	$_end, %eax		# eax = end of bss/start of heap #填入正确的内容
	#生成的栈空间大小为0x1000字节，位于结构的最后
	addl	$STACK_SIZE, %eax	# make room for stack
	andl	$0xffffffc0, %eax	# align it on 16 byte boundary
	
	movl	%eax, %esp		# set stack pointer
	movl	%eax, %ebp		# set base pointer
	
#define uart_base 0x3F8

void uart_put_char(unsigned char c){
	//将字符传入串口
	outb(uart_base, c);
}

unsigned char uart_get_char(void){
	//从串口读出字符   
    return inb(uart_base);     
}

void uart_put_chars(char *str){ 
	//将字符串传入串口    
	for(int i=0;str[i]!='\0';++i){
	    uart_put_char(str[i]);
	}
}

short cur_line=0;
short cur_column=0;//当前光标位置
char * vga_init_p=(char *)0xB8000;

void update_cursor(void){//通过当前行值cur_line与列值cur_column回写光标
	//根据当前的cur_line值与cur_column值，计算出当前的光标值（2字节），并分两次写入端口
	short pos=cur_line*80+cur_column;
	if(pos>=25*80) return;
	outb(0x3D4, 0x0E);
	outb(0x3D5, (unsigned char)((pos&0xFF00)>>8));//输入高8bit
	outb(0x3D4, 0x0F);
	outb(0x3D5, (unsigned char)(pos&0x00FF));//输入低8bit
}

short get_cursor_position(void){//获得当前光标，计算出cur_line和cur_column的值
	//分两次从端口读入当前光标，并计算出当前的cur_line值与cur_column值
	short pos_low, pos_high, pos;
	outb(0x3D4, 0x0E);
	pos_high=inb(0x3D5);
	outb(0x3D4, 0x0F);
	pos_low=inb(0x3D5);
	pos=(pos_high<<8)+pos_low;
	cur_line=pos/80;
	cur_column=pos%80;
	return pos;
}

void clear_screen(void) {
	//将VGA屏幕的25*80个字符（2*25*80个字节）写入黑色的空格字符，并将光标设置在0*0处    
	unsigned char* vga_p=vga_init_p;
	for(int i=0;i<25*80;++i){
	    *vga_p++=' ';
	    *vga_p++=0x00;
	}
	cur_line=0;
	cur_column=0;
	update_cursor();
}

void scroll_screen(){
	//用1~24行的值赋值给0~23行，并将24行清空，将光标设置在24*0处
    unsigned char* vga_p=vga_init_p;
    for(;vga_p<0xB8F00;){
        *vga_p=*(vga_p+80*2);
        vga_p++;
        *vga_p=*(vga_p+80*2);
        vga_p++;
    }
    for(;vga_p<0xB8FA0;){
        *vga_p++=' ';
        *vga_p++=0x00;
    }
    cur_line=24;
    cur_column=0;
    update_cursor();
}

void put_char(unsigned char ch, int color){
	//将颜色为color的字符写入VGA显存，写入地址为当前光标地址
	//若字符为'\n'，则将光标移到下一行行首
	//若光标位置打到25*80，即写满屏幕，进行滚屏
    unsigned char* vga_p=vga_init_p;
    short pos=get_cursor_position();
    if(ch=='\n'){
        ++cur_line;
        cur_column=0;
        pos=cur_line*80+cur_column;
    }
    else if(pos<25*80){
        vga_p+=pos*2;
        *vga_p++=ch;
        *vga_p=(unsigned char)color;
        if(++cur_column==80){
            ++cur_line;
            cur_column=0;
        }
        pos=cur_line*80+cur_column;
    }
    if(pos>=25*80){
        scroll_screen();
    }
    update_cursor();
}

void append2screen(char *str,int color){ 
	//将颜色为color的字符串输入VGA显存    
	for(int i=0;str[i]!='\0';++i){
	    put_char(str[i], color);
	}
}

#include <stdarg.h>  

int vsprintf(char *buf, const char *fmt, va_list argptr){
	//只实现%d的格式化字符串的处理    
    char * str;
	//若没有遇到'%'，输出同样的字符
    for (str=buf ; *fmt ; ++fmt) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }
	//若遇到'%'，根据下一个字符，从argptr中取出对应的变量，改写为字符串加入字符串中
        ++fmt;
        switch (*fmt) {
            case 'd':
                int num=va_arg(argptr, int);
                if(num<0){
                    num=-num;
                    *str++='-';
                }
                char* numstr;
                int i=0;
                while(num>0){
                    int temp=num%10;
                    numstr[i++]=(char)temp+48;
                    num/=10;
                }
                while(i>0){
                    *str++=numstr[--i];
                }
                break;
            default:
                if (*fmt != '%')
                    *str++ = '%';
                if (*fmt)
                    *str++ = *fmt;
                else
                    --fmt;
                break;
        }
    }
    *str = '\0';
    //返回格式化字符串长度
    return str-buf;
}
```

### 代码布局说明

**//代码布局不是这个意思，写错了**（2023年4月29日）

<img src="C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230415212557152.png" alt="image-20230415212557152"  />

### 编译过程说明

输入`./source2run.sh`即可运行脚本，通过脚本运行`qemu-system-i386 -kernel output/myOS.elf -serial stdio`指令，即可运行QEMU，并编译生成`myOS.elf`并运行

### 实验结果

<img src="C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230415212951376.png" alt="image-20230415212951376" style="zoom: 67%;" />

### 遇到的问题和解决方法

无法输出数字0——修改`vsprintf()`函数逻辑

只能滚屏一次——将滚屏函数的循环条件改为按写入的地址的范围

各种其他问题——通过修改`osStart.c`和`main.c`中的函数来定位问题