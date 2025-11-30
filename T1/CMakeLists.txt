cmake_minimum_required(VERSION 3.10)
project(INF1316_T1 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/utils
)

set(SRC_COMMON
    src/fifo.c
    src/process.c
    utils/queue.c
)

add_executable(app
    src/app/app.c
    src/app/app_functions.c
    ${SRC_COMMON}
)

add_executable(intercontroller
    src/intercontroller/intercontroller.c
    ${SRC_COMMON}
)

add_executable(kernel
    src/kernel/kernel.c
    src/kernel/kernel_functions.c
    src/kernel/kernel_reply.c
    src/kernel/udp_client_functions.c
    ${SRC_COMMON}
)
