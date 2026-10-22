cmake_minimum_required(VERSION 3.18)

# Project name
project (dsky VERSION 0.1)

# Debug/Release configuration
set(LOG_LEVEL LOG_DEBUG)

# Add executable
add_executable(dsky "")

# Sources 
target_sources(dsky PRIVATE
  src/main.cpp 
  src/hmi/Hmi.cpp 
  src/hmi/Frame.cpp 
  src/common/Color.cpp 
  src/common/exceptions/DskyException.cpp
  src/common/exceptions/ExcInvalidCommand.cpp
  src/hmi/FrameLeftLcd.cpp 
  src/hmi/FrameCenterLcd.cpp 
  src/hmi/FrameRightLcd.cpp 
  src/hmi/Digit.cpp 
  src/hmi/Number.cpp 
  src/hmi/Label.cpp 
  src/hmi/Draw.cpp 
  src/hmi/Sign.cpp 
  src/hmi/HmiLogic.cpp
  src/hmi/HmiData.cpp
  src/hmi/Font.cpp
  src/hmi/AlarmLabel.cpp
  src/core/Keypad.cpp 
  src/core/Keyboard.cpp
  src/core/Event.cpp
  src/core/StatusMachine.cpp
  src/actions/Action.cpp
  src/actions/ListOfActions.cpp
  src/actions/ActionClock.cpp
  src/actions/EnableKeypadLight.cpp
  src/actions/DisableKeypadLight.cpp
  src/drivers/dskyKeyInt/MsgQueue.cpp
)

# CFLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT")
set(CMAKE_BUILD_TYPE Release)
SET(CMAKE_CXX_FLAGS "-std=c++20")

# Set-up System
# Detect the *target* architecture, not the host: CMAKE_SYSTEM_PROCESSOR is
# set to aarch64 by cmake/pi-aarch64.cmake when cross-compiling for the Pi.
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
  set(SDL_WINDOW_FLAGS SDL_WINDOW_SHOWN)
  message("Using x86_64")
else()
  set(SDL_WINDOW_FLAGS SDL_WINDOW_FULLSCREEN)
  message("Using arm (${CMAKE_SYSTEM_PROCESSOR})")
endif()

# Configuration file
configure_file(config.h.in config.h)

# Directory where the 3rdparty libraries are installed (see 3rdparty/build.sh).
# Native builds install into ./build; Pi cross-builds install into ./build-pi,
# so pass -DDSKY_DEPS_DIR=$PWD/build-pi when building the app for the Pi.
set(DSKY_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build" CACHE PATH "Directory containing the installed 3rdparty libraries (include/, lib/)")

# Include directory
#include_directories(3rdparty/BSlogger/src)
include_directories(${DSKY_DEPS_DIR})
include_directories(${DSKY_DEPS_DIR}/include)
include_directories(${DSKY_DEPS_DIR}/include/SDL2)
include_directories(src/hmi)
include_directories(src/common)
include_directories(src/common/exceptions)
include_directories(src/core)
include_directories(src/actions)
include_directories(3rdparty/simple-cpp-logger/include)
include_directories(src/drivers/dskyKeyInt)

# Generated config.h lives in the build directory
target_include_directories(dsky PRIVATE ${CMAKE_BINARY_DIR})

# Logger definitions
add_definitions( -D LOGGER_MAX_LOG_LEVEL_PRINTED=7 )
add_definitions( -D LOGGER_PREFIX_LEVEL=2 )
add_definitions( -D LOGGER_ENABLE_COLORS=1 )
add_definitions( -D LOGGER_ENABLE_COLORS_ON_USER_HEADER=0 )

# Libraries
target_link_directories(dsky PUBLIC ${DSKY_DEPS_DIR}/lib)
target_link_libraries(dsky SDL2)
target_link_libraries(dsky SDL2_ttf)
target_link_libraries(dsky SDL2_image)
target_link_libraries(dsky SDL2_gfx)
target_link_libraries(dsky pthread)

