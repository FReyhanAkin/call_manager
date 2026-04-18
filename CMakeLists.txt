cmake_minimum_required(VERSION 3.22.1)
project(call_manager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-Wall -Wextra -g)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories(include)

add_executable(call_manager
        main.cpp
        src/node.cpp
        src/heart_beat.cpp
        src/call_state.cpp)

find_package(Threads REQUIRED)
target_link_libraries(call_manager Threads::Threads)