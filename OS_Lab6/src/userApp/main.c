#include "../myOS/userInterface.h"   //interface from kernel
#include "shell.h"
#include "memTestCase.h"

void myTask0(void);
void myTask1(void);
void myTask2(void);
void startShell(void);
void initShell(void);
void memTestCaseInit(void);
void taskEnd(void);

void wallClock_hook_main(void) {
     int _h, _m, _s;
     char hhmmss[] = "hh:mm:ss\0\0\0\0";

     getWallClock(&_h, &_m, &_s);
     sprintf(hhmmss, "%02d:%02d:%02d", _h, _m, _s);
     put_chars(hhmmss, 0x7E, 24, 72);
}

void doSomeTestBefore(void) {
     setWallClock(18, 59, 59);		//set time 18:59:59
     setWallClockHook(&wallClock_hook_main);
     setTickHook(sch.tickHook);
}

// init task 入口
void myMain(void) {
     clear_screen();

     //myPrintf(0x2,"debug_myMain_begin......\n");

     doSomeTestBefore();

     myPrintf(0x07, "********************************\n");
     myPrintf(0x07, "*         INIT   INIT !        *\n");
     myPrintf(0x07, "********************************\n");

     createTask(myTask0, (taskPara) { 3, 180, 10 }); //priority, exeTime, arrTime
     createTask(myTask1, (taskPara) { 2, 200, 0 });
     createTask(myTask2, (taskPara) { 1, 220, 20 });

     //#error "TODO: 初始化 shell 并创建 shell task"

     initShell();
     memTestCaseInit();
     // createTask(startShell, (taskPara) { 0, 0, 0 });

     //myPrintf(0x2,"debug_myMain_end......\n");

     taskEnd();
}
