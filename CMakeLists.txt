cmake_minimum_required(VERSION 3.8.2)
project(Lilc)
add_subdirectory(src)

enable_testing()
add_subdirectory(test)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
