#include "io.h"
#include "myPrintk.h"
#include "uart.h"
#include "vga.h"
#include "i8253.h"
#include "i8259A.h"
#include "tick.h"
#include "wallClock.h"

typedef struct myCommand {
    char name[80];
    char help_content[200];
    int (*func)(int argc, char (*argv)[8]);
}myCommand; 


int func_cmd(int argc, char (*argv)[8]){
	myPrintk(0x02, "cmd\n\0");
	myPrintk(0x02, "help\n\0");
} 

myCommand cmd={"cmd\0","List all command\n\0",func_cmd};

int strcmp(char*a, char*b){
    while(*a&&*b){
        if(*a!=*b) return 0;
        ++a;++b;
    }
    if(!*a&&!*b) return 1;
    return 0;
}
int func_help(int argc, char (*argv)[8]);

myCommand help={"help\0","Usage: help [command]\nDisplay info about [command]\n\0",func_help};

int func_help(int argc, char (*argv)[8]){
    if(argc==2){
        if(strcmp(argv[1], "cmd\0"))
            myPrintk(0x02, cmd.help_content);
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
            }
            else{
                argv[argc-1][j++]=BUF[i];
            }
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

