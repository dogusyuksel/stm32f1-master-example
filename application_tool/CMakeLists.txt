cmake_minimum_required(VERSION 3.1)
project(libcanard)

# Libcanard
include_directories(/workspace/thirdparty/libcanard)
include_directories(/workspace/thirdparty/libcanard/drivers/socketcan)

# Compiler configuration - supporting only Clang and GCC
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -std=c99   -Wall -Wextra -Werror -m32 -pedantic -g -O0")

# C warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wdouble-promotion -Wswitch-enum -Wfloat-equal -Wundef")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wconversion -Wtype-limits")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsign-conversion -Wcast-align -Wmissing-declarations -Wno-conversion -Wno-float-equal")

# Expose internal API for unit testing
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DCANARD_INTERNAL=''")

add_executable(can_dumper.exe
                can_dumper.c
                /workspace/thirdparty/libcanard/canard.c
                /workspace/thirdparty/libcanard/drivers/socketcan/socketcan.c)
