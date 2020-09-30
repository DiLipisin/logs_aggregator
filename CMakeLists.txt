cmake_minimum_required(VERSION 3.10)
project(logs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_subdirectory(lib/jsoncpp)

add_executable(${PROJECT_NAME}
    main.cpp
    input_log_file_handler.cpp input_log_file_handler.h
    balansers.cpp balansers.h
    helpers.cpp helpers.h
    exceptions.h
)

target_link_libraries(${PROJECT_NAME} jsoncpp_lib)
target_link_libraries(${PROJECT_NAME} stdc++fs)
target_link_libraries(${PROJECT_NAME} Threads::Threads)
