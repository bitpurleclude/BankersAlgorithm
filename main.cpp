#include <iostream>

#include <iostream>
#include "Scheduler.h"
#include "generateRandomProcess.h"
int main() {
    // ����һ��������
    Scheduler scheduler;
    scheduler.timeSlice = 100;
    scheduler.finishTime = 0;
    scheduler.resources[0] = 10;
    scheduler.resources[1] = 15;
    scheduler.resources[2] = 12;
    scheduler.processCount = 0;
    scheduler.totalSchedulingTime = 0;

    // ���ɲ�����������
    for (int i = 0; i < 5; ++i) {
        PCB process = generateRandomProcessWithCount(i);
        addProcess(&scheduler, process);
    }

    // ִ�е���
    schedule_round_robin(&scheduler);

    return 0;
}