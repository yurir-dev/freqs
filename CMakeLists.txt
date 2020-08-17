cmake_minimum_required(VERSION 3.10)

set (PROJECT_NAME "concurency")
set (EXE_NAME "concurency")

# set the project name
project(${PROJECT_NAME})

set (SOURCES main.cpp thsQueue.h spinlock_mutex.h tests.h tests.cpp)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the executable
add_executable(${EXE_NAME} ${SOURCES})
