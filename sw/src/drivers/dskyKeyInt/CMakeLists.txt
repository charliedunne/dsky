cmake_minimum_required(VERSION 3.18)

# Project name
project (dskyKeyInt VERSION 0.1)

add_executable(dskyKeyInt main.cpp)

# Includes
include_directories(build/include)

# Libraries
target_link_directories(dskyKeyInt PUBLIC ../../../build/lib)
target_link_libraries(dskyKeyInt pigpio)

