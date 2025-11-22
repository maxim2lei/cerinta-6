#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#ifdef _WIN32
    #include <windows.h>
    #include <stdio.h>
    #include <conio.h>
    #define SEMAPHORE_NAME "CounterSemaphore"
    #define SHARED_MEM_NAME "CounterSharedMemory"
#else
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <semaphore.h>
    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #define SEMAPHORE_NAME "/CounterSemaphore"
    #define SHARED_MEM_NAME "/CounterSharedMemory"
#endif

#include <iostream>
#include <random>
#include <chrono>
#include <thread>

struct SharedData {
    int current_number;
    bool finished;
};

#endif
