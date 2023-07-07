#include "../../include/task.h"
#include "../../include/myPrintk.h"
#include "../../include/wallClock.h"

#define TIME_SLICE 60

//初始化就绪队列
void rqInitRR(myTCB* idleTask) {//对rq进行初始化处理
    rq.idleTask = idleTask;
}    //idleTask不入队

//如果就绪队列为空，返回True
int rqIsEmptyRR(void) {//当head和tail均为NULL时，rq为空
    if (rq.head == NULL && rq.tail == NULL) {
        return 1;
    }
    return 0;
}

//将一个未在就绪队列中的TCB加入到就绪队列中
void taskEnqueueRqRR(myTCB* task) {//将task入队rq
    if (rqIsEmptyRR()) {
        rq.head = task;
        rq.tail = task;
    }
    else {
        rq.tail->rqNextTCB = task;
        rq.tail = task;
    }
    task->rqNextTCB = NULL;
}

//将就绪队列中的TCB移除
void taskDequeueRqRR() {//rq出队
    myTCB* task = rq.head;
    if (rq.head == rq.tail) {
        rq.head = NULL;
        rq.tail = NULL;
    }
    else {
        rq.head = rq.head->rqNextTCB;
    }
    task->rqNextTCB = NULL;
}

//获取就绪队列的头结点信息，并返回
myTCB* rqNextTaskRR(void) {//获取下一个Task
    if (!rq.head) {
        return &tcbPool[1];
    }
    else {
        return rq.head;
    }
}    //若队列为空（如运行0号进程时），返回1号进程的地址

//如果等待队列为空，返回True
int wqIsEmptyRR(void) {//当head和tail均为NULL时，wq为空
    if (wq.head == NULL && wq.tail == NULL) {
        return 1;
    }
    return 0;
}

//将一个未在等待队列中的TCB加入到等待队列中
void taskEnqueueWqRR(myTCB* task) {//将task入队wq
    if (wqIsEmptyRR()) {
        wq.head = task;
        wq.tail = task;
    }
    else {
        wq.tail->wqNextTCB = task;
        wq.tail = task;
    }
    task->wqNextTCB = NULL;
}

//将等待队列中的TCB移除
void taskDequeueWqRR(myTCB* task) {//wq出队
    if (wq.head == wq.tail) {
        if (task != wq.head) {
            return;
        }
        wq.head = NULL;
        wq.tail = NULL;
    }
    if (wq.head == task) {
        wq.head = wq.head->wqNextTCB;
        task->wqNextTCB = NULL;
    }
    myTCB* temp = wq.head;
    while (temp->wqNextTCB) {
        if (temp->wqNextTCB == task) {
            temp->wqNextTCB = temp->wqNextTCB->wqNextTCB;
            task->wqNextTCB = NULL;
            break;
        }
        temp = temp->wqNextTCB;
    }
}

//获取等待队列的头结点信息，并返回
myTCB* wqNextTaskRR(void) {//获取下一个Task
    if (!wq.head) {
        return &tcbPool[1];
    }
    else {
        return wq.head;
    }
}    //若队列为空（如运行0号进程时），返回1号进程的地址

//创建任务并加入等待队列，若到达时间为0则加入就绪队列
void createTaskRR(myTCB* task, void (*taskBody)(void)) {
    task->task_entrance = taskBody;
    stack_init(&(task->stackTop), taskBody);
    task->TASK_State = TASK_WAIT;
    if (task->paras.arrTime == 0) {
        task->TASK_State = TASK_READY;
        taskEnqueueRqRR(task);
    }
    else {
        taskEnqueueWqRR(task);
    }
}

//对于RR调度，当一个时间片结束时，将当前就绪队列队头任务移到队尾，并进行一次调度
void tickHookRR() {
    myTCB* task = wq.head;
    while (task) {
        if (task->TASK_State == TASK_WAIT) {
            task->paras.arrTime--;
            if (task->paras.arrTime == 0) {
                task->TASK_State = TASK_READY;
                taskDequeueWqRR(task);
                taskEnqueueRqRR(task);
            }
        }
        task = task->wqNextTCB;
    }
    if (currentTask->paras.exeTime) {
        currentTask->paras.exeTime--;
    }
    if (rq.head != rq.tail) {
        if (tick_number % TIME_SLICE == 0) {
            task = rq.head;
            taskDequeueRqRR();
            taskEnqueueRqRR(task);
            schedule();
            return;
        }
    }
}

void setSchedulerRR(scheduler* sch) {
    scheduler schedulerRR = {
        .type = RR,
        .preemptiveOrNot = 1,
        .nextTaskFunc = rqNextTaskRR,
        .enqueueTaskFunc = taskEnqueueRqRR,
        .dequeueTaskFunc = taskDequeueRqRR,
        .schedulerInitFunc = rqInitRR,
        .createTaskHook = createTaskRR,
        .tickHook = tickHookRR,
    };
    *sch = schedulerRR;
}
