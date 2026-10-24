cmake_minimum_required(VERSION 3.18)

# Project name (must be "Font": the runner finds the only executable at the
# build dir root)
project (Font VERSION 1.0)

# Debug/Release configuration
set(LOG_LEVEL LOG_DEBUG)

# Absolute resource dir so GORTON_FONT_* in the generated config.h resolve
# correctly in this standalone test build.
set(DSKY_RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../resources)

# Generated config.h (Font.h includes "config.h")
configure_file(../../config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)

# Add executable
add_executable(Font "")

# Sources
target_sources(Font PRIVATE
  main.cpp
  ../../src/hmi/Font.cpp
  ../../src/common/Color.cpp
)

# CFLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

# Include directory
include_directories(../../)
include_directories(../../src/hmi)
include_directories(../../src/common)
include_directories(../../build/include)
include_directories(../../3rdparty/simple-cpp-logger/include)
# Generated config.h
include_directories(${CMAKE_BINARY_DIR})

# Logger definitions
add_definitions( -D LOGGER_MAX_LOG_LEVEL_PRINTED=7 )
add_definitions( -D LOGGER_PREFIX_LEVEL=2 )
add_definitions( -D LOGGER_ENABLE_COLORS=1 )
add_definitions( -D LOGGER_ENABLE_COLORS_ON_USER_HEADER=0 )

# Libraries
target_link_directories(Font PUBLIC ../../build/lib)
target_link_libraries(Font SDL2)
target_link_libraries(Font SDL2_image)
