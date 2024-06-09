//
// Created by 15517 on 24-5-19.
//

#ifndef PROCESSSCHEDULING_GENERATERANDOMPROCESS_H
#define PROCESSSCHEDULING_GENERATERANDOMPROCESS_H

PCB generateRandomProcessWithCount(int processCount) {
    srand(time(NULL)); // 初始化随机数生成器

    // 生成随机的到达时间、所需时间和优先级
    int arrivalTime = processCount == 0 ? 0 : rand() % 1000;
    int requiredTime = rand() % 1000;
    int priority = rand() % 1000;

    // 生成随机的资源需求
    int resourcesNeeded[3];
    for (int i = 0; i < 3; ++i) {
        resourcesNeeded[i] = rand() % 10 + 1;
    }

    // 创建一个新的PCB对象
    PCB process;
    process.arrivalTime = time(NULL) + arrivalTime;
    process.requiredTime = requiredTime;
    process.priority = priority;
    process.elapsedTime = 0;
    process.processState = Wait;
    sprintf(process.processName, "Process %d", processCount + 1); // 设置进程名
    memcpy(process.resourcesNeeded, resourcesNeeded, sizeof(resourcesNeeded)); // 设置资源需求

    return process;
}


#endif //PROCESSSCHEDULING_GENERATERANDOMPROCESS_H
