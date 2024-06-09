#include <iostream>

#include <iostream>
#include "Scheduler.h"
#include "generateRandomProcess.h"
int main() {
    // 创建一个调度器
    Scheduler scheduler;
    scheduler.timeSlice = 100;
    scheduler.finishTime = 0;
    scheduler.resources[0] = 10;
    scheduler.resources[1] = 15;
    scheduler.resources[2] = 12;
    scheduler.processCount = 0;
    scheduler.totalSchedulingTime = 0;

    // 生成并添加随机进程
    for (int i = 0; i < 5; ++i) {
        PCB process = generateRandomProcessWithCount(i);
        addProcess(&scheduler, process);
    }

    // 执行调度
    schedule_round_robin(&scheduler);

    return 0;
}