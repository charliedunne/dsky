cmake_minimum_required(VERSION 3.20)

# Project name
project (dsky VERSION 0.1)

# Debug/Release configuration
set(LOG_LEVEL LOG_DEBUG)

# Add executable
add_executable(dsky src/main.cpp src/hmi/src/Hmi.cpp src/hmi/src/Frame.cpp)

# Configuration file
configure_file(config.h.in config.h)

# Include directory
include_directories(3rdparty/BSlogger/src)
include_directories(./build)
include_directories(./build/include)
include_directories(./src/hmi/include)

# Libraries
target_link_directories(dsky PUBLIC ./build/lib)
target_link_libraries(dsky SDL2)
