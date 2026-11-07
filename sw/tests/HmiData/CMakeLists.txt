cmake_minimum_required(VERSION 3.18)

# Project name
project (hmiData VERSION 1.0)

# Add executable
add_executable(hmiData "")

# Sources
target_sources(hmiData PRIVATE
  main.cpp
  ../../src/hmi/HmiData.cpp
  ../../src/hmi/VerbListView.cpp
  ../../src/hmi/MoonPhaseView.cpp
  ../../src/hmi/TempView.cpp
  ../common/frame_stub.cpp
)

# CFLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

# Generated config.h (needed by Label.h in the VerbListView -> FrameLeftLcd.h chain)
configure_file(../../config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)

# Include directory
include_directories(../../)
include_directories(../../src/hmi)
include_directories(../../src/common)
include_directories(../../3rdparty/simple-cpp-logger/include)
# SDL2 headers only (VerbListView.cpp includes FrameLeftLcd.h); never link SDL
include_directories(../../build/include)
# Generated config.h
include_directories(${CMAKE_BINARY_DIR})

# Logger definitions
add_definitions( -D LOGGER_MAX_LOG_LEVEL_PRINTED=7 )
add_definitions( -D LOGGER_PREFIX_LEVEL=2 )
add_definitions( -D LOGGER_ENABLE_COLORS=1 )
add_definitions( -D LOGGER_ENABLE_COLORS_ON_USER_HEADER=0 )

# Libraries
target_link_libraries(hmiData pthread)
