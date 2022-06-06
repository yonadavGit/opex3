cmake_minimum_required(VERSION 3.22)
project(opex3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(opex3 main.cpp)
