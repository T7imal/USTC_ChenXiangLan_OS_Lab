#include "../include/task.h"
#include "../include/myPrintk.h"

void schedule(void);
void destroyTask(unsigned long taskIndex);

#define TASK_READY 0     //表示当前进程已经进入就绪队列中
#define TASK_WAIT -1     //表示当前进程还未进入就绪队列中
#define TASK_RUNNING 1   //表示当前进程正在运行
#define TASK_NONE 2      //表示进程池中的TCB为空未进行分配
#define NULL (void*)0

myTCB* idleTask;         /* idle 任务 */
myTCB* currentTask;      /* 当前任务 */
myTCB* firstFreeTask;    /* 下一个空闲的 TCB */
myTCB tcbPool[TASK_NUM]; //进程池的大小设置

//taskIdleBdy进程（无需填写）
void taskIdleBody(void) {
     while (1) {
          schedule();
     }
}

//taskEmpty进程（无需填写）
void taskEmpty(void) {
}

//就绪队列的结构体
typedef struct readyQueueFCFS {
     myTCB* head;
     myTCB* tail;
     myTCB* idleTask;
} readyQueueFCFS;

readyQueueFCFS rqFCFS;

//初始化就绪队列（需要填写）
void rqFCFSInit(myTCB* idleTask) {//对rqFCFS进行初始化处理
     rqFCFS.idleTask = idleTask;
}

//如果就绪队列为空，返回True（需要填写）
int rqFCFSIsEmpty(void) {//当head和tail均为NULL时，rqFCFS为空
     if (rqFCFS.head == NULL && rqFCFS.tail == NULL) {
          return 1;
     }
     return 0;
}

//获取就绪队列的头结点信息，并返回（需要填写）
myTCB* nextFCFSTask(void) {//获取下一个Task
     if (!rqFCFS.head) {
          return &tcbPool[1];
     }
     return rqFCFS.head;
}

//将一个未在就绪队列中的TCB加入到就绪队列中（需要填写）
void taskEnqueueFCFS(myTCB* task) {//将task入队rqFCFS
     if (rqFCFSIsEmpty()) {
          rqFCFS.head = task;
          rqFCFS.tail = task;
     }
     else {
          rqFCFS.tail->nextTCB = task;
          rqFCFS.tail = task;
     }
}

//将就绪队列中的TCB移除（需要填写）
void taskDequeueFCFS(myTCB* task) {//rqFCFS出队
     rqFCFS.head = rqFCFS.head->nextTCB;
}

//初始化栈空间（不需要填写）
void stack_init(unsigned long** stack, void (*task)(void)) {
     *(*stack)-- = (unsigned long)0x08;       //高地址
     *(*stack)-- = (unsigned long)task;       //EIP
     *(*stack)-- = (unsigned long)0x0202;     //FLAG寄存器

     *(*stack)-- = (unsigned long)0xAAAAAAAA; //EAX
     *(*stack)-- = (unsigned long)0xCCCCCCCC; //ECX
     *(*stack)-- = (unsigned long)0xDDDDDDDD; //EDX
     *(*stack)-- = (unsigned long)0xBBBBBBBB; //EBX

     *(*stack)-- = (unsigned long)0x44444444; //ESP
     *(*stack)-- = (unsigned long)0x55555555; //EBP
     *(*stack)-- = (unsigned long)0x66666666; //ESI
     *(*stack) = (unsigned long)0x77777777; //EDI

}

//进程池中一个未在就绪队列中的TCB的开始（不需要填写）
void taskStart(myTCB* task) {
     task->TASK_State = TASK_READY;
     //将一个未在就绪队列中的TCB加入到就绪队列
     taskEnqueueFCFS(task);
}

//进程池中一个在就绪队列中的TCB的结束（不需要填写）
void taskEnd(void) {
     //将一个在就绪队列中的TCB移除就绪队列
     taskDequeueFCFS(currentTask);
     //由于TCB结束，我们将进程池中对应的TCB也删除
     destroyTask(currentTask->TASK_ID);
     //TCB结束后，我们需要进行一次调度
     schedule();
}

//以taskBody为参数在进程池中创建一个进程，并调用taskStart函数，将其加入就绪队列（需要填写）
int createTask(void (*taskBody)(void)) {//在进程池中创建一个进程，并把该进程加入到rqFCFS队列中
     myTCB* task = firstFreeTask;
     task->task_entrance = taskBody;
     stack_init(&(task->stackTop), taskBody);
     taskStart(task);
     for (int i = 0;i < TASK_NUM;++i) {
          if (tcbPool[i].TASK_State == TASK_NONE) {
               firstFreeTask = &tcbPool[i];
               break;
          }
     }
     return task->TASK_ID;
}

//以taskIndex为关键字，在进程池中寻找并销毁taskIndex对应的进程（需要填写）
void destroyTask(unsigned long taskIndex) {//在进程中寻找TASK_ID为taskIndex的进程，并销毁该进程
     int i = 0;
     for (i = 0;i < TASK_NUM;++i) {
          if (tcbPool[i].TASK_ID == taskIndex) {
               break;
          }
     }
     myTCB* task = &tcbPool[i];
     task->TASK_State = TASK_NONE;
}

unsigned long** prevTASK_StackPtr;
unsigned long* nextTASK_StackPtr;

//切换上下文（无需填写）
void context_switch(myTCB* prevTask, myTCB* nextTask) {
     prevTASK_StackPtr = &(prevTask->stackTop);
     currentTask = nextTask;
     nextTASK_StackPtr = nextTask->stackTop;
     CTX_SW(prevTASK_StackPtr, nextTASK_StackPtr);
}

//FCFS调度算法（无需填写）
void scheduleFCFS(void) {
     myTCB* nextTask;
     nextTask = nextFCFSTask();
     context_switch(currentTask, nextTask);
}

//调度算法（无需填写）
void schedule(void) {
     scheduleFCFS();
}

//进入多任务调度模式(无需填写)
unsigned long BspContextBase[STACK_SIZE];
unsigned long* BspContext;
void startMultitask(void) {
     BspContext = BspContextBase + STACK_SIZE - 1;
     prevTASK_StackPtr = &BspContext;
     //currentTask = nextFCFSTask();
     currentTask = &tcbPool[0];
     nextTASK_StackPtr = currentTask->stackTop;
     CTX_SW(prevTASK_StackPtr, nextTASK_StackPtr);
}

//准备进入多任务调度模式(无需填写)
void TaskManagerInit(void) {
     // 初始化进程池（所有的进程状态都是TASK_NONE）
     int i;
     myTCB* thisTCB;
     for (i = 0;i < TASK_NUM;i++) {//对进程池tcbPool中的进程进行初始化处理
          thisTCB = &tcbPool[i];
          thisTCB->TASK_ID = i;
          thisTCB->stackTop = thisTCB->stack + STACK_SIZE - 1;//将栈顶指针复位
          thisTCB->TASK_State = TASK_NONE;//表示该进程池未分配，可用
          thisTCB->task_entrance = taskEmpty;
          if (i == TASK_NUM - 1) {
               thisTCB->nextTCB = (void*)0;
          }
          else {
               thisTCB->nextTCB = &tcbPool[i + 1];
          }
     }
     //创建idle任务
     idleTask = &tcbPool[0];
     stack_init(&(idleTask->stackTop), taskIdleBody);
     idleTask->task_entrance = taskIdleBody;
     idleTask->nextTCB = (void*)0;
     idleTask->TASK_State = TASK_READY;
     rqFCFSInit(idleTask);

     firstFreeTask = &tcbPool[1];

     //创建init任务
     createTask(initTaskBody);

     //进入多任务状态
     myPrintk(0x2, "START MULTITASKING......\n");
     startMultitask();
     myPrintk(0x2, "STOP MULTITASKING......SHUT DOWN\n");

}
