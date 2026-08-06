cmake_minimum_required(VERSION 4.2)
project(MCMS)

set(CMAKE_CXX_STANDARD 23)

add_executable(MCMS
        main.cpp
        FileLoad.cpp
        PreSchedulerQueue.cpp
        PostSchedulerQueue.cpp
        wait.cpp
        Executor.cpp
        cpu_timer.cpp
)

include_directories(include _3rd)

include(CTest)

if (BUILD_TESTING)
    add_executable(MCMS_tests
            tests.cpp
            PreSchedulerQueue.cpp
            PostSchedulerQueue.cpp
            wait.cpp
            Executor.cpp
            cpu_timer.cpp
    )

    add_test(NAME MCMS_tests COMMAND MCMS_tests)
    set_tests_properties(MCMS_tests PROPERTIES TIMEOUT 15)
endif()
