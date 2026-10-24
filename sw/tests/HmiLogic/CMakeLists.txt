cmake_minimum_required(VERSION 3.18)

# Project name
project (HmiLogic VERSION 1.0)

# Debug/Release configuration
set(LOG_LEVEL LOG_DEBUG)

# Add executable
add_executable(HmiLogic "")

# Sources
target_sources(HmiLogic PRIVATE
  main.cpp
  ../../src/hmi/HmiLogic.cpp
  ../../src/hmi/HmiData.cpp
  ../../src/core/StatusMachine.cpp
  ../../src/core/Event.cpp
  ../../src/actions/Action.cpp
  ../../src/actions/ListOfActions.cpp
  ../../src/actions/ActionVerbList.cpp
  ../../src/common/exceptions/DskyException.cpp
  ../../src/common/exceptions/ExcInvalidCommand.cpp
)

# CFLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

# Generated config.h (needed by Font.h in the HmiLogic -> Hmi.h include chain)
configure_file(../../config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)

# Include directory
include_directories(../../)
include_directories(../../src/hmi)
include_directories(../../src/core)
include_directories(../../src/actions)
include_directories(../../src/common)
include_directories(../../src/common/exceptions)
include_directories(../../3rdparty/simple-cpp-logger/include)
# SDL2 headers only (Frame.h includes <SDL2/SDL.h>); never link SDL
include_directories(../../build/include)
# Generated config.h
include_directories(${CMAKE_BINARY_DIR})

# Logger definitions
add_definitions( -D LOGGER_MAX_LOG_LEVEL_PRINTED=7 )
add_definitions( -D LOGGER_PREFIX_LEVEL=2 )
add_definitions( -D LOGGER_ENABLE_COLORS=1 )
add_definitions( -D LOGGER_ENABLE_COLORS_ON_USER_HEADER=0 )

# Libraries
target_link_libraries(HmiLogic pthread)
