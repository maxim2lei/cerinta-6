#include "shared_memory.h"

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 1);

#ifdef _WIN32
    // Windows implementation
    HANDLE hSemaphore = CreateSemaphoreA(
        NULL,           // default security attributes
        1,              // initial count
        1,              // maximum count
        SEMAPHORE_NAME  // named semaphore
    );
    
    if (hSemaphore == NULL) {
        std::cerr << "CreateSemaphore failed: " << GetLastError() << std::endl;
        return 1;
    }

    HANDLE hMapFile = CreateFileMappingA(
        INVALID_HANDLE_VALUE,    // use paging file
        NULL,                    // default security
        PAGE_READWRITE,          // read/write access
        0,                       // maximum object size (high-order DWORD)
        sizeof(SharedData),      // maximum object size (low-order DWORD)
        SHARED_MEM_NAME          // name of mapping object
    );

    if (hMapFile == NULL) {
        std::cerr << "CreateFileMapping failed: " << GetLastError() << std::endl;
        CloseHandle(hSemaphore);
        return 1;
    }

    SharedData* pSharedData = (SharedData*)MapViewOfFile(
        hMapFile,            // handle to map object
        FILE_MAP_ALL_ACCESS, // read/write permission
        0,
        0,
        sizeof(SharedData)
    );

    if (pSharedData == NULL) {
        std::cerr << "MapViewOfFile failed: " << GetLastError() << std::endl;
        CloseHandle(hMapFile);
        CloseHandle(hSemaphore);
        return 1;
    }

    // Initialize shared memory
    pSharedData->current_number = 0;
    pSharedData->finished = false;

    std::cout << "Process 1 started. Counting from 1 to 1000..." << std::endl;

    while (true) {
        // Wait for semaphore (acquire lock)
        DWORD dwWaitResult = WaitForSingleObject(hSemaphore, INFINITE);
        
        if (dwWaitResult == WAIT_OBJECT_0) {
            // Critical section: read from shared memory
            int current = pSharedData->current_number;
            
            if (current >= 1000) {
                pSharedData->finished = true;
                ReleaseSemaphore(hSemaphore, 1, NULL);
                break;
            }

            // Flip coin: keep flipping while we get 1 (heads)
            bool can_write = false;
            int coin;
            do {
                coin = distrib(gen);
                if (coin == 1) {
                    can_write = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            } while (coin == 0);

            // If we won the coin flip, write next number
            if (can_write && current < 1000) {
                pSharedData->current_number = current + 1;
                std::cout << "Process 1 wrote: " << (current + 1) << std::endl;
            }

            // Release semaphore
            ReleaseSemaphore(hSemaphore, 1, NULL);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "Process 1 finished!" << std::endl;

    // Cleanup
    UnmapViewOfFile(pSharedData);
    CloseHandle(hMapFile);
    CloseHandle(hSemaphore);

#else
    // Linux/POSIX implementation
    sem_t* semaphore = sem_open(SEMAPHORE_NAME, O_CREAT, 0644, 1);
    if (semaphore == SEM_FAILED) {
        perror("sem_open failed");
        return 1;
    }

    int shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        sem_close(semaphore);
        return 1;
    }

    if (ftruncate(shm_fd, sizeof(SharedData)) == -1) {
        perror("ftruncate failed");
        close(shm_fd);
        sem_close(semaphore);
        return 1;
    }

    SharedData* pSharedData = (SharedData*)mmap(
        NULL,
        sizeof(SharedData),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm_fd,
        0
    );

    if (pSharedData == MAP_FAILED) {
        perror("mmap failed");
        close(shm_fd);
        sem_close(semaphore);
        return 1;
    }

    // Initialize shared memory
    pSharedData->current_number = 0;
    pSharedData->finished = false;

    std::cout << "Process 1 started. Counting from 1 to 1000..." << std::endl;

    while (true) {
        // Wait for semaphore (acquire lock)
        sem_wait(semaphore);
        
        // Critical section: read from shared memory
        int current = pSharedData->current_number;
        
        if (current >= 1000) {
            pSharedData->finished = true;
            sem_post(semaphore);
            break;
        }

        // Flip coin: keep flipping while we get 1 (heads)
        bool can_write = false;
        int coin;
        do {
            coin = distrib(gen);
            if (coin == 1) {
                can_write = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } while (coin == 0);

        // If we won the coin flip, write next number
        if (can_write && current < 1000) {
            pSharedData->current_number = current + 1;
            std::cout << "Process 1 wrote: " << (current + 1) << std::endl;
        }

        // Release semaphore
        sem_post(semaphore);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "Process 1 finished!" << std::endl;

    // Cleanup
    munmap(pSharedData, sizeof(SharedData));
    close(shm_fd);
    sem_close(semaphore);
#endif

    return 0;
}
