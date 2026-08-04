cmake_minimum_required(VERSION 4.2)
project(MCMS)

set(CMAKE_CXX_STANDARD 23)

add_executable(MCMS
        main.cpp
        FileLoad.cpp
        ActionQueue.cpp
)

include_directories(include)
