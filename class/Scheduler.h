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
    PCB processes[MAX_PROCESSES];  // �洢���еĽ���
    int processCount;
    double timeSlice;  // ʱ��Ƭ����
    double finishTime;
    double totalSchedulingTime;
    int resources[3];  // ϵͳ�п��õ�3����Դ������
} Scheduler;

void addProcess(Scheduler *scheduler, PCB process) {
    scheduler->processes[scheduler->processCount++] = process;
}

// ��ӡ���н��̵�״̬
void printProcesses(Scheduler *scheduler) {
    printf("=====================================================================================================\n");
    for (int i = 0; i < scheduler->processCount; i++) {
        PCB process = scheduler->processes[i];
        char *state;
        switch (process.processState) {
            case Wait:
                state = "�ȴ�";
                break;
            case Run:
                state = "����";
                break;
            case Block:
                state = "����";
                break;
            case Finish:
                state = "���";
                break;
        }

        printf("��������: %s, ״̬: %s, ������ʱ��: %.2f, ����ʱ��: %.2f, ��Ӧ��: %.2f, ��Ҫ����Դ: [%d, %d, %d], �ѷ������Դ: [%d, %d, %d]\n",
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
        // ��鱻�����Ľ��̣������Ƿ��н��̿��Խ������
        for (int i = 0; i < blockedQueueCount; i++) {
            PCB *process = blockedQueue[i];
            int request[3];
            for (int j = 0; j < 3; ++j) {
                request[j] = process->resourcesNeeded[j];
            }
            if (isSafe(scheduler, request, process)) {
                processQueue[processQueueCount++] = process;
                // �������������Ƴ�
                memmove(&blockedQueue[i], &blockedQueue[i + 1], (blockedQueueCount - i - 1) * sizeof(PCB *));
                blockedQueueCount--;
                process->processState = Wait;
            }
        }

        if (processQueueCount == 0) {
            continue;  // ���û�п������еĽ��̣����������ѭ��
        }

        PCB *process = processQueue[0];
        // �ӽ��̶������Ƴ�
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
                        // �ͷŽ����ѷ������Դ
                        for (int i = 0; i < 3; ++i) {
                            scheduler->resources[i] += process->resourcesAllocated[i];
                            process->resourcesAllocated[i] = 0;
                        }
                        double schedulingTime = difftime(time(NULL), process->startTime);  // �������ʱ��
                        scheduler->totalSchedulingTime += schedulingTime;  // ��ӵ��ܵĵ���ʱ��
                    } else {
                        process->elapsedTime += scheduler->timeSlice;  // ����������ʱ��
                        process->requiredTime -= scheduler->timeSlice;
                        sleep_for(scheduler->timeSlice);
                        process->processState = Run;
                        double schedulingTime = difftime(time(NULL), process->startTime);  // �������ʱ��
                        scheduler->totalSchedulingTime += schedulingTime;  // ��ӵ��ܵĵ���ʱ��
                    }
                }
            } else {
                process->processState = Block;
            }

            if (process->processState == Block) {
                blockedQueue[blockedQueueCount++] = process;  // ������̱��������ͽ�����ӵ���������
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

        // �ӽ����б����Ƴ�
        memmove(&scheduler->processes[0], &scheduler->processes[1], (scheduler->processCount - 1) * sizeof(PCB));
        scheduler->processCount--;
    }
    printProcesses(scheduler);
    printf("Average Scheduling Time: %.2f\n", scheduler->totalSchedulingTime / scheduler->processCount);
}


#endif //PROCESSSCHEDULING_SCHEDULER_H