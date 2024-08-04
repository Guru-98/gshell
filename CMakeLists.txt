project("gshell")
cmake_minimum_required(VERSION 3.15)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "-g")

add_executable(gshell gshell.c)
