//
// Created by 15517 on 24-5-19.
//

#ifndef PROCESSSCHEDULING_PCB_H
#define PROCESSSCHEDULING_PCB_H


#include <string>
#include <chrono>

typedef enum { Wait, Run, Block, Finish } State;

typedef struct {
    double requiredTime; // 进程所需的运行时间
    State processState; // 进程的状态
    double elapsedTime; // 进程已经运行的时间
    double responseRatio; // 进程的响应比
    char processName[50]; // 进程的名称
    time_t arrivalTime; // 进程的到达时间
    int priority; // 进程的优先级
    time_t startTime;
    int resourcesNeeded[3]; // 进程需要的资源总数
    int resourcesAllocated[3]; // 已分配的资源数
} PCB;

#endif //PROCESS SCHEDULING_PCB_H
