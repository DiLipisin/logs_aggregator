cmake_minimum_required(VERSION 3.10)
project(logs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_subdirectory(lib/jsoncpp)

file(GLOB SOURCES src/*.cpp src/*.h)
add_executable(${PROJECT_NAME} main.cpp ${SOURCES} src/utils.cpp src/utils.h)

target_link_libraries(${PROJECT_NAME} jsoncpp_lib)
target_link_libraries(${PROJECT_NAME} stdc++fs)
target_link_libraries(${PROJECT_NAME} Threads::Threads)
