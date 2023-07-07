#include "../include/task.h"

//初始化任务参数
void initTaskPara(taskPara* buffer) {
    buffer->priority = 0;
    buffer->exeTime = 0;
    buffer->arrTime = 0;
}

//设置任务参数
void setTaskPara(unsigned int option, unsigned int value, taskPara* buffer) {
    switch (option) {
    case PRIORITY:
        buffer->priority = value;
        break;
    case EXETIME:
        buffer->exeTime = value;
        break;
    case ARRTIME:
        buffer->arrTime = value;
        break;
    default:
        break;
    }
}

//获取任务参数
unsigned int getTaskPara(unsigned int option, taskPara* buffer) {
    switch (option) {
    case PRIORITY:
        return buffer->priority;
        break;
    case EXETIME:
        return buffer->exeTime;
        break;
    case ARRTIME:
        return buffer->arrTime;
        break;
    default:
        return 0;
        break;
    }
}


