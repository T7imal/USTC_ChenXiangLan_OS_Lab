# OS Lab6 实验报告

### 实验目标

1. 在Lab5的基础上，实现FCFS、SJF、优先级、RR四种调度方法，并将其模块化封装

### 源代码说明

**由于本次实验没有提供框架，所以源代码比较混乱，在报告中仅展示关键部分**

`task.c`文件中，原Lab5中实现的`FCFS`函数，被替代为`scheduler`类型（与文档中大致相同）的调度器函数，而对应算法的实现则位于`taskScheduler`文件夹中，实现了模块化封装，如下所示

```c
typedef struct scheduler {
     unsigned int type;
     int preemptiveOrNot;     //if True, the scheduler is preemptive
     myTCB* (*nextTaskFunc)(void);
     void(*enqueueTaskFunc)(myTCB* task);
     void(*dequeueTaskFunc)();
     void(*schedulerInitFunc)(myTCB* task);
     void(*createTaskHook)(myTCB* task, void (*taskBody)(void));    //if set, will be called in createTask (before taskStart)
     void(*tickHook)(void);   //if set, tickHook will be called every tick
}scheduler;

//进程池中一个未在就绪队列中的TCB的开始
void taskStart(myTCB* task) {
    task->TASK_State = TASK_READY;
    //将一个未在就绪队列中的TCB加入到就绪队列
    sch.enqueueTaskFunc(task);
}
```

调度方法的设置位于`OSStart.c`文件中，在`TaskManagerInit()`函数前进行

```c
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
```

在`myYCB`类型中增加优先级、执行时间和到达时间参数

```c
// struct for taskPara
typedef struct taskPara {
     unsigned int priority;
     unsigned int exeTime;
     unsigned int arrTime;
} taskPara;

//#error "TODO: 为 myTCB 增补合适的字段"
typedef struct myTCB {
     unsigned long* stackTop;        /* 栈顶指针 */
     unsigned long stack[STACK_SIZE];      /* 开辟了一个大小为STACK_SIZE的栈空间 */
     unsigned long TASK_State;   /* 进程状态 */
     unsigned long TASK_ID;      /* 进程ID */
     taskPara paras;
     void (*task_entrance)(void);  /*进程的入口地址*/
     struct myTCB* rqNextTCB;      /*就绪队列中下一个TCB*/
     struct myTCB* wqNextTCB;      /*等待队列中下一个TCB*/
} myTCB;
```

为实现到达时间和执行时间，利用`tickHook()`函数，每`tick`运行一次，将等待队列中的任务的到达时间减一，将当前执行的任务的执行时间减一；当等待队列中任意任务的到达时间为零时，将其移出等待队列，移进就绪队列

另外创建任务时，若到达时间为0，则直接进入就绪队列

**对于RR调度，当一个时间片结束时，将当前就绪队列队头任务移到队尾，并进行一次调度**

```c
void tickHookFCFS() {
    myTCB* task = wq.head;
    while (task) {
        if (task->TASK_State == TASK_WAIT) {
            task->paras.arrTime--;
            if (task->paras.arrTime == 0) {
                task->TASK_State = TASK_READY;
                taskDequeueWqFCFS(task);
                taskEnqueueRqFCFS(task);
            }
        }
        task = task->wqNextTCB;
    }
    if (currentTask->paras.exeTime) {
        currentTask->paras.exeTime--;
    }
}

//创建任务并加入等待队列，若到达时间为0则加入就绪队列
void createTaskFCFS(myTCB* task, void (*taskBody)(void)) {
    task->task_entrance = taskBody;
    stack_init(&(task->stackTop), taskBody);
    task->TASK_State = TASK_WAIT;
    if (task->paras.arrTime == 0) {
        task->TASK_State = TASK_READY;
        taskEnqueueRqFCFS(task);
    }
    else {
        taskEnqueueWqFCFS(task);
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
        }
    }
}
```

在优先级和SJF调度中，我选择在任务进入就绪队列时就已优先级进行排序，这样调度时只需要将下一个任务设为当前就绪队列头即可

**当入队的任务的优先级比当前任务的优先级更高（执行时间比当前任务更短）时，抢占当前任务**

```c
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
```

### 实验运行结果

为方便展示，删去了shell任务

创建的任务如图所示，每当`tick_number`是10的倍数时，输出当前`tick_number`，共输出10次。可能是由于`tick_number`变化太快，程序在两次`tick`之间无法运行完一次循环，所以不是每一次`tick_number`是10的倍数时，都能够输出当前`tick_number`，当输出10次时，能够保证任务执行时间在1s与2s之间

三个任务输出的颜色分别为绿（0x2）、紫（0x5）、蓝（0x3）

```c
void myTask0(void) {
	myPrintf(WHITE, message1);
	myPrintf(WHITE, "*     Task0: TASK BEGIN!       *\n");
	int times = 0, temp = 0;
	while (times < 10) {
		if (temp != tick_number) {
			if (tick_number % 10 == 0) {
				myPrintf(COLOR0, "%d ", tick_number);
				times++;
			}
		}
		temp = tick_number;
	}
	myPrintf(WHITE, "\n");
	myPrintf(RED, "*     Task0: TASK END!       *\n");
	myPrintf(RED, message1);

	taskEnd();   //the task is end
}
```

设置三个任务的执行时间均为200（2s），到达时间如图所示

```c
createTask(myTask0, (taskPara) { 3, 190, 30 }); //priority, exeTime, arrTime
createTask(myTask1, (taskPara) { 2, 200, 0 });
createTask(myTask2, (taskPara) { 1, 210, 60 });
```

**FCFS**

符合到达时间的顺序

![image-20230707164730162](C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230707164730162.png)

**SJF**

30时间单位时，Task0进入就绪队列，执行时间为190，而此时Task1执行时间为200-30=170，因此无抢占。同理，Task2也无抢占

![image-20230707164706241](C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230707164706241.png)

**优先级**

首先执行最先进入就绪队列的Task1，Task2在60时间单位时抢占Task1开始执行。200时间单位后，Task2执行完毕，Task1剩余部分开始执行。140时间单位后，优先级最低的Task0开始执行

![image-20230707163836358](C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230707163836358.png)

**RR**

时间片设置为60时间单位

![image-20230707163737187](C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230707163737187.png)

将测试任务改为

```c
createTask(myTask0, (taskPara) { 3, 180, 10 }); //priority, exeTime, arrTime
createTask(myTask1, (taskPara) { 2, 200, 0 });
createTask(myTask2, (taskPara) { 1, 220, 20 });
```

使用SJF调度，即可体现出抢占（Task0抢占Task1）

![image-20230707164904540](C:\Users\hwc\AppData\Roaming\Typora\typora-user-images\image-20230707164904540.png)

### 遇到的问题和解决方法

在设置调度器之前就调用了调度器中的函数，导致程序卡死