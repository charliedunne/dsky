cmake_minimum_required(VERSION 3.18)

# Project name
project (keystub VERSION 0.1)

# Exec rule
add_executable(keystub "")

# Sources
target_sources(keystub PRIVATE
    main.cpp
)

# Includes
include_directories(build)
include_directories(./)
include_directories(../../src/drivers/dskyKeyInt/)
include_directories(../../src/core/)

# Libraries
#target_link_directories(dskyKeyInt PUBLIC ../../../build/lib)
#target_link_libraries(dskyKeyInt pigpio)

