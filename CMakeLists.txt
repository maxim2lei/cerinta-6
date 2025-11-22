cmake_minimum_required(VERSION 3.10)
project(untitled2)

set(CMAKE_CXX_STANDARD 20)

# Definește executabilele cu calea corectă
add_executable(process1
        src/shared_memory.h
        src/process1.cpp)

add_executable(process2
        src/shared_memory.h
        src/process2.cpp)

# Link libraries doar pentru UNIX (Linux)
if(UNIX)
    target_link_libraries(process1 pthread rt)
    target_link_libraries(process2 pthread rt)
endif()
