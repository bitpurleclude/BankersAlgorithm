//
// Created by 15517 on 24-5-19.
//

#ifndef PROCESSSCHEDULING_PCB_H
#define PROCESSSCHEDULING_PCB_H


#include <string>
#include <chrono>

enum class State { Wait, Run, Block, Finish };

class PCB {
public:
    std::chrono::duration<double> requiredTime; // 进程所需的运行时间
    State processState; // 进程的状态
    std::chrono::duration<double> elapsedTime; // 进程已经运行的时间
    double responseRatio; // 进程的响应比
    std::string processName; // 进程的名称
    std::chrono::system_clock::time_point arrivalTime; // 进程的到达时间
    int priority; // 进程的优先级
    std::chrono::time_point<std::chrono::system_clock> startTime;
    int resourcesNeeded[3]; // 进程需要的资源总数
    int resourcesAllocated[3] = {0, 0, 0}; // 已分配的资源数

//    PCB(const std::string& name, int resourcesNeeded[3])
//            : processName(name), processState(State::Wait) {
//        std::copy(resourcesNeeded, resourcesNeeded + 3, this->resourcesNeeded);
//    }
};
//        while (!processQueue.empty()) {
//            PCB *process = processQueue.front();
//
//            if (process->processState == State::Wait || process->processState == State::Run) {
//                int request[3];
//                for (int i = 0; i < 3; ++i) {
//                    request[i] = std::min(rand() % (process->resourcesNeeded[i] - process->resourcesAllocated[i] + 1),
//                                          resources[i]);
//                }
//
//                if (isSafe(request, process)) {
//                    for (int i = 0; i < 3; ++i) {
//                        resources[i] -= request[i];
//                        process->resourcesAllocated[i] += request[i];
//                    }
//
//                    if (std::equal(process->resourcesNeeded, process->resourcesNeeded + 3,
//                                   process->resourcesAllocated)) {
//                        process->processState = State::Finish;
//                        // 释放进程已分配的资源
//                        for (int i = 0; i < 3; ++i) {
//                            resources[i] += process->resourcesAllocated[i];
//                            process->resourcesAllocated[i] = 0;
//                        }
//                    } else {
//                        process->processState = State::Run;
//                    }
//                } else {
//                    process->processState = State::Block;
//                }
//
//                if (process->processState != State::Finish) {
//                    processQueue.push_back(process);
//                }
//            }
//
//            processQueue.pop_front();
//            printProcesses();
//        }


#endif //PROCESS SCHEDULING_PCB_H
