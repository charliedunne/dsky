cmake_minimum_required(VERSION 3.18)

# Project name
project (dsky VERSION 0.1)

# Debug/Release configuration
set(LOG_LEVEL LOG_DEBUG)

# Add executable
add_executable(dsky src/main.cpp src/hmi/src/Hmi.cpp src/hmi/src/Frame.cpp src/common/Color.cpp src/hmi/src/FrameRightLcd.cpp)

# CFLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT")

# Configuration file
configure_file(config.h.in config.h)

# Include directory
#include_directories(3rdparty/BSlogger/src)
include_directories(build)
include_directories(build/include)
include_directories(build/include/SDL2)
include_directories(src/hmi/include)
include_directories(src/common)
include_directories(3rdparty/simple-cpp-logger/include)

# Logger definitions
add_definitions( -D LOGGER_MAX_LOG_LEVEL_PRINTED=1 )
add_definitions( -D LOGGER_PREFIX_LEVEL=2 )
add_definitions( -D LOGGER_ENABLE_COLORS=1 )
add_definitions( -D LOGGER_ENABLE_COLORS_ON_USER_HEADER=0 )

# Libraries
target_link_directories(dsky PUBLIC ./build/lib)
target_link_libraries(dsky SDL2)
target_link_libraries(dsky SDL2_gfx)
