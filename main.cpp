#include <iostream>

#include <iostream>
#include "Scheduler.h"
#include "generateRandomProcess.h"

int main() {
    // 创建一个随机进程生成器
    generateRandomProcess generator;

    // 创建一个调度器
    Scheduler scheduler(std::chrono::milliseconds(100), std::chrono::milliseconds(0), {10, 15, 12});

    // 生成并添加随机进程
    for (int i = 0; i < 5; ++i) {
        PCB process = generator.generateRandomProcessWithCount(i);
        scheduler.addProcess(process);
    }

    // 执行调度
    scheduler.schedule_round_robin();

    return 0;
}
