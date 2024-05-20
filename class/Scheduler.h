//
// Created by 15517 on 24-5-19.
//

#ifndef PROCESSSCHEDULING_SCHEDULER_H
#define PROCESSSCHEDULING_SCHEDULER_H

#include <vector>
#include <algorithm>
#include <thread>
#include <iostream>
#include <deque>
#include <array>
#include "PCB.h"

using namespace std;

class Scheduler {
    std::vector<PCB> processes;  // 存储所有的进程
    std::chrono::duration<double> timeSlice{};  // 时间片长度
    std::chrono::duration<double> finishTime{};
    std::chrono::duration<double> totalSchedulingTime{};
    int resources[3]{};  // 系统中可用的3种资源的数量

public:
    Scheduler(std::chrono::duration<double> timeSlice, std::chrono::duration<double> finishTime,
              std::array<int, 3> resources) {
        this->timeSlice = timeSlice;
        this->finishTime = finishTime;
        std::copy(resources.begin(), resources.end(), this->resources);
    }

    void addProcess(const PCB &process) {
        processes.push_back(process);
    }

    // 打印所有进程的状态
    void printProcesses() {
        for (const PCB &process: processes) {
            std::string state;
            switch (process.processState) {
                case State::Wait:
                    state = "Wait";
                    break;
                case State::Run:
                    state = "Run";
                    break;
                case State::Block:
                    state = "Block";
                    break;
                case State::Finish:
                    state = "Finish";
                    break;
            }

            std::cout << "Process Name: " << process.processName
                      << ", State: " << state
                      << ", Elapsed Time: " << process.elapsedTime.count()
                      << ", Required Time: " << process.requiredTime.count()
                      << ", Response Ratio: " << process.responseRatio
                      << ", Resources Needed: [" << process.resourcesNeeded[0] << ", " << process.resourcesNeeded[1] << ", " << process.resourcesNeeded[2] << "]"
                      << ", Resources Allocated: [" << process.resourcesAllocated[0] << ", " << process.resourcesAllocated[1] << ", " << process.resourcesAllocated[2] << "]"
                      << std::endl;
        }
    }

    void schedule_shortest_job_first() {
        std::sort(processes.begin(), processes.end(), [](const PCB &a, const PCB &b) {
            if (a.requiredTime == b.requiredTime) {
                return a.priority < b.priority;
            }
            return a.requiredTime < b.requiredTime;
        });

        for (PCB &process: processes) {
            if (process.processState == State::Wait || process.processState == State::Run) {
                process.processState = State::Run;
                process.startTime = std::chrono::system_clock::now();
                while (process.requiredTime > timeSlice) {
                    process.elapsedTime += timeSlice;
                    process.requiredTime -= timeSlice;
                    std::this_thread::sleep_for(timeSlice);
                }
                process.processState = State::Finish;
                std::chrono::duration<double> schedulingTime = std::chrono::system_clock::now() - process.startTime;
                totalSchedulingTime += schedulingTime;
                process.elapsedTime += process.requiredTime;
                process.requiredTime = finishTime;
            }
            printProcesses();
        }
        std::cout << "Average Scheduling Time: " << totalSchedulingTime.count() / processes.size() << std::endl;
    }

    void schedule_round_robin() {
        std::deque<PCB *> processQueue;
        std::deque<PCB *> blockedQueue;
        for (PCB &process: processes) {
            processQueue.push_back(&process);
        }
        while ( !processQueue.empty()||!blockedQueue.empty()) {
            // 检查被阻塞的进程，看看是否有进程可以解除阻塞
            for (auto it = blockedQueue.begin(); it != blockedQueue.end(); ) {
                PCB *process = *it;
                int request[3];
                for (int i = 0; i < 3; ++i) {
                    request[i] = process->resourcesNeeded[i];
                }
                if (isSafe(request, process)) {
                    processQueue.push_back(process);
                    it = blockedQueue.erase(it);  // 从阻塞队列中移除
                    process->processState = State::Wait;
                } else {
                    ++it;
                }
            }

            if (processQueue.empty()) {
                continue;  // 如果没有可以运行的进程，就跳过这次循环
            }

            PCB *process = processQueue.front();
            processQueue.pop_front();

            if ( process->processState != State::Finish) {
                int request[3];
                for (int i = 0; i < 3; ++i) {
                    request[i] = std::min(rand() % (process->resourcesNeeded[i] - process->resourcesAllocated[i] + 1),
                                          resources[i]);
                }

                if (isSafe(request, process)) {
                    for (int i = 0; i < 3; ++i) {
                        resources[i] -= request[i];
                        process->resourcesAllocated[i] += request[i];
                    }

                    if (std::equal(process->resourcesNeeded, process->resourcesNeeded + 3,
                                   process->resourcesAllocated)&&process->requiredTime<=timeSlice) {
                        process->processState = State::Finish;
                        std::chrono::duration<double> schedulingTime = std::chrono::system_clock::now() - process->startTime;  // 计算调度时间
                        totalSchedulingTime += schedulingTime;  // 添加到总的调度时间
                        process->elapsedTime += process->requiredTime;
                        process->requiredTime = finishTime;
                        // 释放进程已分配的资源
                        for (int i = 0; i < 3; ++i) {
                            resources[i] += process->resourcesAllocated[i];
                            process->resourcesAllocated[i] = 0;
                        }
                    } else {
                        process->elapsedTime += timeSlice;  // 增加已运行时间
                        process->requiredTime -= timeSlice;
                        std::this_thread::sleep_for(timeSlice);
                        process->processState = State::Run;
                    }
                } else {
                    process->processState = State::Block;
                }

                if (process->processState == State::Block) {
                    blockedQueue.push_back(process);  // 如果进程被阻塞，就将其添加到阻塞队列
                } else if (process->processState != State::Finish) {
                    processQueue.push_back(process);
                }
            }



            printProcesses();
        }

        std::cout << "Average Scheduling Time: " << totalSchedulingTime.count() / processes.size() << std::endl;
    }

    bool isSafe(int request[3], PCB *process) {
        // Step 1: Create a work vector and initialize it to the current number of available resources.
        int work[3];
        for (int i = 0; i < 3; ++i) {
            work[i] = resources[i] -request[i];
            process->resourcesAllocated[i] += request[i];
        }
        std::vector<bool> finish(processes.size(), false);
        while (true) {
            bool found = false;
            for (size_t i = 0; i < processes.size(); ++i) {

                if(processes[i].processState==State::Finish){
                    finish[i]=true;
                }else if (!finish[i] && isEnoughtResources(processes[i], work)) {
                    for (int j = 0; j < 3; ++j) {
                        work[j] += processes[i].resourcesAllocated[j];
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
        return std::all_of(finish.begin(), finish.end(), [](bool b) { return b; });
    }

    bool isEnoughtResources(PCB pcb,int work[]) {
        for (int i = 0; i < 3; ++i) {
            if (pcb.resourcesNeeded[i]-pcb.resourcesAllocated[i]>work[i]) {
                return false;
            }
        }
        return true;
    }


    void schedule_highest_response_ratio_next() {
        for (PCB &process: processes) {
            std::chrono::duration<double> waitingTime = process.elapsedTime;
            process.responseRatio = (waitingTime + process.requiredTime) / process.requiredTime;
        }

        while (!processes.empty()) {
            std::sort(processes.begin(), processes.end(), [](const PCB &a, const PCB &b) {
                return a.responseRatio > b.responseRatio;
            });

            PCB &process = processes.front();
            process.processState = State::Run;
            process.startTime = std::chrono::system_clock::now();
            process.elapsedTime += process.requiredTime;
            process.requiredTime = finishTime;
            process.processState = State::Finish;
            std::chrono::duration<double> schedulingTime = std::chrono::system_clock::now() - process.startTime;
            totalSchedulingTime += schedulingTime;

            processes.erase(processes.begin());
        }
        printProcesses();
        std::cout << "Average Scheduling Time: " << totalSchedulingTime.count() / processes.size() << std::endl;
    }
};

#endif //PROCESSSCHEDULING_SCHEDULER_H