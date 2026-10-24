cmake_minimum_required(VERSION 3.18)

# Project name
project (listOfActions VERSION 1.0)

# Debug/Release configuration
set(LOG_LEVEL LOG_DEBUG)

# Add executable
add_executable(listOfActions "")

# Sources
target_sources(listOfActions PRIVATE
  main.cpp
  ../../src/actions/Action.cpp
  ../../src/actions/ListOfActions.cpp
  ../../src/actions/ActionVerbList.cpp
  ../../src/hmi/HmiData.cpp
  ../../src/common/exceptions/DskyException.cpp
  ../../src/common/exceptions/ExcInvalidCommand.cpp
)

# CFLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

# Include directory
include_directories(../../)
include_directories(../../src/actions)
include_directories(../../src/hmi)
include_directories(../../src/common/exceptions)
include_directories(../../3rdparty/simple-cpp-logger/include)

# Logger definitions
add_definitions( -D LOGGER_MAX_LOG_LEVEL_PRINTED=7 )
add_definitions( -D LOGGER_PREFIX_LEVEL=2 )
add_definitions( -D LOGGER_ENABLE_COLORS=1 )
add_definitions( -D LOGGER_ENABLE_COLORS_ON_USER_HEADER=0 )

# Libraries
target_link_libraries(listOfActions pthread)
