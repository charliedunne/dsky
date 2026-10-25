cmake_minimum_required(VERSION 3.18)

# Project name
project (event VERSION 1.0)

# Add executable
add_executable(event "")

# Sources
target_sources(event PRIVATE
  main.cpp
  ../../src/core/Event.cpp
)

# CFLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

# Include directory
include_directories(../../)
include_directories(../../src/core)

# Libraries
target_link_libraries(event pthread)
