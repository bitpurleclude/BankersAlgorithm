cmake_minimum_required(VERSION 3.28)
project(BankersAlgorithm)

set(CMAKE_CXX_STANDARD 17)
include_directories(${PROJECT_SOURCE_DIR}/class)
add_executable(BankersAlgorithm main.cpp
        class/generateRandomProcess.h
        class/PCB.h
        class/Scheduler.h)
