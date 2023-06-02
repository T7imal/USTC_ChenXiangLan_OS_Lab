#ifndef __TASK_H__
#define __TASK_H__

#ifndef USER_TASK_NUM
#include "../../userApp/userApp.h"
#endif

#define TASK_NUM (2 + USER_TASK_NUM)   // at least: 0-idle, 1-init

#define initTaskBody myMain         // connect initTask with myMain

#define STACK_SIZE 512            // definition of STACK_SIZE

void initTaskBody(void);

void CTX_SW(void* prev_stackTop, void* next_stackTop);

//#error "TODO: 为 myTCB 增补合适的字段"
typedef struct myTCB {
     unsigned long* stackTop;        /* 栈顶指针 */
     unsigned long stack[STACK_SIZE];      /* 开辟了一个大小为STACK_SIZE的栈空间 */
     unsigned long TASK_State;   /* 进程状态 */
     unsigned long TASK_ID;      /* 进程ID */
     void (*task_entrance)(void);  /*进程的入口地址*/
     struct myTCB* nextTCB;           /*下一个TCB*/
} myTCB;

extern myTCB tcbPool[TASK_NUM];    //进程池的大小设置

extern myTCB* idleTask;            /* idle 任务 */
extern myTCB* currentTask;         /* 当前任务 */
extern myTCB* firstFreeTask;       /* 下一个空闲的 TCB */

void TaskManagerInit(void);

#endif
