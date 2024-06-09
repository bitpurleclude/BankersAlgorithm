//
// Created by 15517 on 24-5-19.
//

#ifndef PROCESSSCHEDULING_SCHEDULER_H
#define PROCESSSCHEDULING_SCHEDULER_H

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include "PCB.h"

using namespace std;
#define MAX_PROCESSES 100
typedef struct {
    PCB processes[MAX_PROCESSES];  // 存储所有的进程
    int processCount;
    double timeSlice;  // 时间片长度
    double finishTime;
    double totalSchedulingTime;
    int resources[3];  // 系统中可用的3种资源的数量
} Scheduler;

void addProcess(Scheduler *scheduler, PCB process) {
    scheduler->processes[scheduler->processCount++] = process;
}

// 打印所有进程的状态
void printProcesses(Scheduler *scheduler) {
    printf("=====================================================================================================\n");
    for (int i = 0; i < scheduler->processCount; i++) {
        PCB process = scheduler->processes[i];
        char *state;
        switch (process.processState) {
            case Wait:
                state = "等待";
                break;
            case Run:
                state = "运行";
                break;
            case Block:
                state = "阻塞";
                break;
            case Finish:
                state = "完成";
                break;
        }

        printf("进程名称: %s, 状态: %s, 已运行时间: %.2f, 所需时间: %.2f, 响应比: %.2f, 需要的资源: [%d, %d, %d], 已分配的资源: [%d, %d, %d]\n",
               process.processName, state, process.elapsedTime, process.requiredTime, process.responseRatio,
               process.resourcesNeeded[0], process.resourcesNeeded[1], process.resourcesNeeded[2],
               process.resourcesAllocated[0], process.resourcesAllocated[1], process.resourcesAllocated[2]);
    }
}

bool isEnoughtResources(PCB *pcb, int work[]) {
    for (int i = 0; i < 3; ++i) {
        if (pcb->resourcesNeeded[i] - pcb->resourcesAllocated[i] > work[i]) {
            return false;
        }
    }
    return true;
}

bool isSafe(Scheduler *scheduler, int request[3], PCB *process) {
    // Step 1: Create a work vector and initialize it to the current number of available resources.
    int work[3];
    for (int i = 0; i < 3; ++i) {
        work[i] = scheduler->resources[i] - request[i];
        process->resourcesAllocated[i] += request[i];
    }
    bool finish[MAX_PROCESSES] = {false};
    while (true) {
        bool found = false;
        for (int i = 0; i < scheduler->processCount; ++i) {
            if (scheduler->processes[i].processState == Finish) {
                finish[i] = true;
            } else if (!finish[i] && isEnoughtResources(&scheduler->processes[i], work)) {
                for (int j = 0; j < 3; ++j) {
                    work[j] += scheduler->processes[i].resourcesAllocated[j];
                }
                finish[i] = true;
                found = true;
            }
        }
        if (!found) {
            break;
        }
    }
    for (int i = 0; i < 3; ++i) {
        process->resourcesAllocated[i] -= request[i];
    }
    for (int i = 0; i < scheduler->processCount; ++i) {
        if (!finish[i]) {
            return false;
        }
    }
    return true;
}

void sleep_for(double time) {
    usleep(time * 1000);
}

int comparePCB(const void *a, const void *b) {
    PCB *pcbA = (PCB *) a;
    PCB *pcbB = (PCB *) b;
    return pcbA->requiredTime - pcbB->requiredTime;
}

void schedule_shortest_job_first(Scheduler *scheduler) {
    qsort(scheduler->processes, scheduler->processCount, sizeof(PCB), comparePCB);

    for (int i = 0; i < scheduler->processCount; i++) {
        PCB *process = &scheduler->processes[i];
        if (process->processState == Wait || process->processState == Run) {
            process->processState = Run;
            process->startTime = time(NULL);
            while (process->requiredTime > scheduler->timeSlice) {
                process->elapsedTime += scheduler->timeSlice;
                process->requiredTime -= scheduler->timeSlice;
                sleep_for(scheduler->timeSlice);
            }
            process->processState = Finish;
            double schedulingTime = difftime(time(NULL), process->startTime);
            scheduler->totalSchedulingTime += schedulingTime;
            process->elapsedTime += process->requiredTime;
            process->requiredTime = scheduler->finishTime;
        }
        printProcesses(scheduler);
    }
    printf("Average Scheduling Time: %.2f\n", scheduler->totalSchedulingTime / scheduler->processCount);
}

void schedule_round_robin(Scheduler *scheduler) {
    PCB *processQueue[MAX_PROCESSES];
    int processQueueCount = 0;
    PCB *blockedQueue[MAX_PROCESSES];
    int blockedQueueCount = 0;
    for (int i = 0; i < scheduler->processCount; i++) {
        processQueue[processQueueCount++] = &scheduler->processes[i];
    }
    while (processQueueCount > 0 || blockedQueueCount > 0) {
        // 检查被阻塞的进程，看看是否有进程可以解除阻塞
        for (int i = 0; i < blockedQueueCount; i++) {
            PCB *process = blockedQueue[i];
            int request[3];
            for (int j = 0; j < 3; ++j) {
                request[j] = process->resourcesNeeded[j];
            }
            if (isSafe(scheduler, request, process)) {
                processQueue[processQueueCount++] = process;
                // 从阻塞队列中移除
                memmove(&blockedQueue[i], &blockedQueue[i + 1], (blockedQueueCount - i - 1) * sizeof(PCB *));
                blockedQueueCount--;
                process->processState = Wait;
            }
        }

        if (processQueueCount == 0) {
            continue;  // 如果没有可以运行的进程，就跳过这次循环
        }

        PCB *process = processQueue[0];
        // 从进程队列中移除
        memmove(&processQueue[0], &processQueue[1], (processQueueCount - 1) * sizeof(PCB *));
        processQueueCount--;

        if (process->processState != Finish) {
            int request[3];
            for (int i = 0; i < 3; ++i) {
                request[i] = rand() % (process->resourcesNeeded[i] - process->resourcesAllocated[i] + 1);
                if (request[i] > scheduler->resources[i]) {
                    request[i] = scheduler->resources[i];
                }
            }

            if (isSafe(scheduler, request, process)) {
                for (int i = 0; i < 3; ++i) {
                    scheduler->resources[i] -= request[i];
                    process->resourcesAllocated[i] += request[i];
                }
                process->startTime = time(NULL);

                if (memcmp(process->resourcesNeeded, process->resourcesAllocated, sizeof(process->resourcesNeeded)) ==0) {
                    if (process->requiredTime <= scheduler->timeSlice){
                        process->processState = Finish;
                        process->elapsedTime += process->requiredTime;
                        process->requiredTime = scheduler->finishTime;
                        // 释放进程已分配的资源
                        for (int i = 0; i < 3; ++i) {
                            scheduler->resources[i] += process->resourcesAllocated[i];
                            process->resourcesAllocated[i] = 0;
                        }
                        double schedulingTime = difftime(time(NULL), process->startTime);  // 计算调度时间
                        scheduler->totalSchedulingTime += schedulingTime;  // 添加到总的调度时间
                    } else {
                        process->elapsedTime += scheduler->timeSlice;  // 增加已运行时间
                        process->requiredTime -= scheduler->timeSlice;
                        sleep_for(scheduler->timeSlice);
                        process->processState = Run;
                        double schedulingTime = difftime(time(NULL), process->startTime);  // 计算调度时间
                        scheduler->totalSchedulingTime += schedulingTime;  // 添加到总的调度时间
                    }
                }
            } else {
                process->processState = Block;
            }

            if (process->processState == Block) {
                blockedQueue[blockedQueueCount++] = process;  // 如果进程被阻塞，就将其添加到阻塞队列
            } else if (process->processState != Finish) {
                processQueue[processQueueCount++] = process;
            }
        }

        printProcesses(scheduler);
    }

    printf("Average Scheduling Time: %.2f\n", scheduler->totalSchedulingTime / scheduler->processCount);
}

int comparePCBResponseRatio(const void *a, const void *b) {
    PCB *pcbA = (PCB *) a;
    PCB *pcbB = (PCB *) b;
    return pcbA->responseRatio < pcbB->responseRatio;
}

void schedule_highest_response_ratio_next(Scheduler *scheduler) {
    for (int i = 0; i < scheduler->processCount; i++) {
        PCB *process = &scheduler->processes[i];
        double waitingTime = process->elapsedTime;
        process->responseRatio = (waitingTime + process->requiredTime) / process->requiredTime;
    }

    qsort(scheduler->processes, scheduler->processCount, sizeof(PCB), comparePCBResponseRatio);

    while (scheduler->processCount > 0) {
        PCB *process = &scheduler->processes[0];
        process->processState = Run;
        process->startTime = time(NULL);
        process->elapsedTime += process->requiredTime;
        process->requiredTime = scheduler->finishTime;
        process->processState = Finish;
        double schedulingTime = difftime(time(NULL), process->startTime);
        scheduler->totalSchedulingTime += schedulingTime;

        // 从进程列表中移除
        memmove(&scheduler->processes[0], &scheduler->processes[1], (scheduler->processCount - 1) * sizeof(PCB));
        scheduler->processCount--;
    }
    printProcesses(scheduler);
    printf("Average Scheduling Time: %.2f\n", scheduler->totalSchedulingTime / scheduler->processCount);
}


#endif //PROCESSSCHEDULING_SCHEDULER_H