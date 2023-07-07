#include "../include/task.h"
#include "../include/myPrintk.h"

void schedule(void);
void destroyTask(unsigned long taskIndex);
void setSchedulerFCFS(scheduler* sch);
void setSchedulerPrio(scheduler* sch);
void setSchedulerSJF(scheduler* sch);

myTCB* idleTask;         /* idle 任务 */
myTCB* currentTask;      /* 当前任务 */
myTCB* firstFreeTask;    /* 下一个空闲的 TCB */
myTCB tcbPool[TASK_NUM]; //进程池的大小设置
scheduler sch;
readyQueue rq;
waitingQueue wq;

void setScheduler(int scheduleMethod) {
    switch (scheduleMethod) {
    case FCFS:
        setSchedulerFCFS(&sch);
        break;
    case PRIO:
        setSchedulerPrio(&sch);
        break;
    case SJF:
        setSchedulerSJF(&sch);
        break;
    case RR:
        setSchedulerRR(&sch);
    default:
        myPrintk(0x02, "Invalid scheduler type.");
        break;
    }
}

//taskIdleBdy进程
void taskIdleBody(void) {
    schedule();
}

//初始化就绪队列
void rqInit(myTCB* idleTask) {//对rq进行初始化处理
    sch.schedulerInitFunc(idleTask);
}

void taskEmpty() {
}

//检查就绪队列是否为空
int isRqEmpty(void) {
    if (!rq.head && !rq.tail) {
        return 1;
    }
    return 0;
}

//初始化栈空间
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

//进程池中一个在就绪队列中的TCB的结束
void taskEnd(void) {
    //将一个在就绪队列中的TCB移除就绪队列
    sch.dequeueTaskFunc(currentTask);
    //由于TCB结束，我们将进程池中对应的TCB也删除
    destroyTask(currentTask->TASK_ID);
    //TCB结束后，我们需要进行一次调度
    //当前任务执行时间结束后再调度
    while (currentTask->paras.exeTime);
    schedule();
}

//进程池中一个未在就绪队列中的TCB的开始
void taskStart(myTCB* task) {
    task->TASK_State = TASK_READY;
    //将一个未在就绪队列中的TCB加入到就绪队列
    sch.enqueueTaskFunc(task);
}

//以taskBody为参数在进程池中创建一个进程，并调用taskStart函数，将其加入等待队列，若等待时间为0则加入就绪队列
int createTask(void (*taskBody)(void), taskPara paras) {
    myTCB* task = firstFreeTask;
    setTaskPara(PRIORITY, paras.priority, &(task->paras));
    setTaskPara(EXETIME, paras.exeTime, &(task->paras));
    setTaskPara(ARRTIME, paras.arrTime, &(task->paras));
    sch.createTaskHook(task, taskBody);
    for (int i = 0;i < TASK_NUM;++i) {
        if (tcbPool[i].TASK_State == TASK_NONE) {
            firstFreeTask = &tcbPool[i];
            break;
        }
    }
    return task->TASK_ID;
}	//在firstFreeTask地址创建新进程，初始化栈空间，编辑进入函数，并入队
     //遍历地址空间，找到第一个空闲的进程块，以其地址更新firstFreeTask
     //（返回值没有用到？）

//以taskIndex为关键字，在进程池中寻找并销毁taskIndex对应的进程
void destroyTask(unsigned long taskIndex) {
    int i = 0;
    for (i = 0;i < TASK_NUM;++i) {
        if (tcbPool[i].TASK_ID == taskIndex) {
            break;
        }
    }
    myTCB* task = &tcbPool[i];
    task->TASK_State = TASK_NONE;
}	//遍历地址空间，找到第一个ID相同的进程块，将其状态设为空闲

unsigned long** prevTASK_StackPtr;
unsigned long* nextTASK_StackPtr;

//切换上下文
void context_switch(myTCB* prevTask, myTCB* nextTask) {
    prevTASK_StackPtr = &(prevTask->stackTop);
    currentTask = nextTask;
    nextTASK_StackPtr = nextTask->stackTop;
    CTX_SW(prevTASK_StackPtr, nextTASK_StackPtr);
}

//调度算法
void schedule(void) {
    while (isRqEmpty());
    myTCB* nextTask;
    nextTask = sch.nextTaskFunc();
    context_switch(currentTask, nextTask);
}

//进入多任务调度模式
unsigned long BspContextBase[STACK_SIZE];
unsigned long* BspContext;
void startMultitask(void) {
    BspContext = BspContextBase + STACK_SIZE - 1;
    prevTASK_StackPtr = &BspContext;
    currentTask = &tcbPool[0];
    nextTASK_StackPtr = currentTask->stackTop;
    CTX_SW(prevTASK_StackPtr, nextTASK_StackPtr);
}

//准备进入多任务调度模式
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
        initTaskPara(&(thisTCB->paras));
        thisTCB->rqNextTCB = (void*)0;
        thisTCB->wqNextTCB = (void*)0;
    }
    //创建idle任务
    idleTask = &tcbPool[0];
    stack_init(&(idleTask->stackTop), taskIdleBody);
    idleTask->task_entrance = taskIdleBody;
    idleTask->TASK_State = TASK_READY;
    rqInit(idleTask);

    firstFreeTask = &tcbPool[1];

    //创建init任务
    createTask(initTaskBody, (taskPara) { 0, 0, 0 });
    //进入多任务状态
    myPrintk(0x2, "START MULTITASKING......\n");
    startMultitask();
    myPrintk(0x2, "STOP MULTITASKING......SHUT DOWN\n");

}
