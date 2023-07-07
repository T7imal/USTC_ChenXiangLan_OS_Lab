#include "../../include/task.h"
#include "../../include/myPrintk.h"

//初始化就绪队列
void rqInitPrio(myTCB* idleTask) {//对rq进行初始化处理
    rq.idleTask = idleTask;
}    //idleTask不入队

//如果就绪队列为空，返回True
int rqIsEmptyPrio(void) {//当head和tail均为NULL时，rq为空
    if (rq.head == NULL && rq.tail == NULL) {
        return 1;
    }
    return 0;
}

//将一个未在就绪队列中的TCB按优先数升序加入到就绪队列中
void taskEnqueueRqPrio(myTCB* task) {//将task入队rq
    if (rqIsEmptyPrio()) {
        rq.head = task;
        rq.tail = task;
    }
    else if (rq.head == rq.tail) {
        if (task->paras.priority < rq.head->paras.priority) {
            task->rqNextTCB = rq.head;
            rq.head = task;
            schedule(); //当入队的任务的优先级比当前任务的优先级更高时，抢占当前任务
            return;
        }
        else {
            rq.tail->rqNextTCB = task;
            task->rqNextTCB = NULL;
            rq.tail = task;
        }
    }
    else {
        if (task->paras.priority < rq.head->paras.priority) {
            task->rqNextTCB = rq.head;
            rq.head = task;
            schedule(); //当入队的任务的优先级比当前任务的优先级更高时，抢占当前任务
            return;
        }
        myTCB* temp = rq.head;
        while (temp->rqNextTCB) {
            if (task->paras.priority < temp->rqNextTCB->paras.priority) {
                task->rqNextTCB = temp->rqNextTCB;
                temp->rqNextTCB = task;
                break;
            }
            temp = temp->rqNextTCB;
        }
        if (!(temp->rqNextTCB)) {
            temp->rqNextTCB = task;
            task->rqNextTCB = NULL;
            rq.tail = task;
        }
    }
}

//将就绪队列中的TCB移除
void taskDequeueRqPrio() {//rq出队
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
myTCB* rqNextTaskPrio(void) {//获取下一个Task
    if (!rq.head) {
        return &tcbPool[1];
    }
    else {
        return rq.head;
    }
}    //若队列为空（如运行0号进程时），返回1号进程的地址

//如果等待队列为空，返回True
int wqIsEmptyPrio(void) {//当head和tail均为NULL时，wq为空
    if (wq.head == NULL && wq.tail == NULL) {
        return 1;
    }
    return 0;
}

//将一个未在等待队列中的TCB加入到等待队列中
void taskEnqueueWqPrio(myTCB* task) {//将task入队wq
    if (wqIsEmptyPrio()) {
        wq.head = task;
        wq.tail = task;
    }
    else if (wq.head == wq.tail) {
        if (task->paras.priority < wq.head->paras.priority) {
            task->wqNextTCB = wq.head;
            wq.head = task;
        }
        else {
            wq.tail->wqNextTCB = task;
            task->wqNextTCB = NULL;
            wq.tail = task;
        }
    }
    else {
        if (task->paras.priority < wq.head->paras.priority) {
            task->wqNextTCB = wq.head;
            wq.head = task;
        }
        myTCB* temp = wq.head;
        while (temp->wqNextTCB) {
            if (task->paras.priority < temp->wqNextTCB->paras.priority) {
                task->wqNextTCB = temp->wqNextTCB;
                temp->wqNextTCB = task;
                break;
            }
            temp = temp->wqNextTCB;
        }
        if (!(temp->wqNextTCB)) {
            temp->wqNextTCB = task;
            task->wqNextTCB = NULL;
            wq.tail = task;
        }
    }
}

//将等待队列中的TCB移除
void taskDequeueWqPrio(myTCB* task) {//wq出队
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
myTCB* wqNextTaskPrio(void) {//获取下一个Task
    if (!wq.head) {
        return &tcbPool[1];
    }
    else {
        return wq.head;
    }
}    //若队列为空（如运行0号进程时），返回1号进程的地址

//创建任务并加入等待队列，若到达时间为0则加入就绪队列
void createTaskPrio(myTCB* task, void (*taskBody)(void)) {
    task->task_entrance = taskBody;
    stack_init(&(task->stackTop), taskBody);
    task->TASK_State = TASK_WAIT;
    if (task->paras.arrTime == 0) {
        task->TASK_State = TASK_READY;
        taskEnqueueRqPrio(task);
    }
    else {
        taskEnqueueWqPrio(task);
    }
}

void tickHookPrio() {
    myTCB* task = wq.head;
    while (task) {
        if (task->TASK_State == TASK_WAIT) {
            task->paras.arrTime--;
            if (task->paras.arrTime == 0) {
                task->TASK_State = TASK_READY;
                taskDequeueWqPrio(task);
                taskEnqueueRqPrio(task);
            }
        }
        task = task->wqNextTCB;
    }
    if (currentTask->paras.exeTime) {
        currentTask->paras.exeTime--;
    }
}

void setSchedulerPrio(scheduler* sch) {
    scheduler schedulerPrio = {
        .type = PRIO,
        .preemptiveOrNot = 1,
        .nextTaskFunc = rqNextTaskPrio,
        .enqueueTaskFunc = taskEnqueueRqPrio,
        .dequeueTaskFunc = taskDequeueRqPrio,
        .schedulerInitFunc = rqInitPrio,
        .createTaskHook = createTaskPrio,
        .tickHook = tickHookPrio,
    };
    *sch = schedulerPrio;
}
