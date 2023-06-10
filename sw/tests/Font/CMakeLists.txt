cmake_minimum_required(VERSION 3.18)

# Project name
project (font VERSION 1.0)

# Debug/Release configuration
set(LOG_LEVEL LOG_DEBUG)

# Add executable
add_executable(font "")

# Sources
target_sources(font PRIVATE
  main.cpp
  ../../src/hmi/Font.cpp
)

# CFLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT")
set(CMAKE_BUILD_TYPE Release)
SET(CMAKE_CXX_FLAGS "-std=c++20")

# Include directory
include_directories(../../)
include_directories(../../src/hmi)
include_directories(../../build/include/SDL2)
include_directories(../../build)
include_directories(../../build/include)
include_directories(../../3rdparty/simple-cpp-logger/include)

# Logger definitions
add_definitions( -D LOGGER_MAX_LOG_LEVEL_PRINTED=7 )
add_definitions( -D LOGGER_PREFIX_LEVEL=2 )
add_definitions( -D LOGGER_ENABLE_COLORS=1 )
add_definitions( -D LOGGER_ENABLE_COLORS_ON_USER_HEADER=0 )

# Libraries
target_link_directories(font PUBLIC ../../build/lib)
target_link_libraries(font SDL2)
target_link_libraries(font SDL2_image)
target_link_libraries(font SDL2_gfx)
