cmake_minimum_required(VERSION 3.18)

# Project name
project (dskyKeyInt VERSION 0.1)

# Configuration file
configure_file(config.h.in config.h)

# Exec rule
add_executable(dskyKeyInt "")

# Sources
target_sources(dskyKeyInt PRIVATE
    main.cpp
    KeyInt.cpp
    Pcf8575.cpp
)

# Includes
include_directories(build)
include_directories(build/include)
include_directories(../../../build/include)
include_directories(../../../3rdparty/simple-cpp-logger/include)

# Libraries
target_link_directories(dskyKeyInt PUBLIC ../../../build/lib)
target_link_libraries(dskyKeyInt pigpio)

# Logger definitions
add_definitions( -D LOGGER_MAX_LOG_LEVEL_PRINTED=7 )
add_definitions( -D LOGGER_PREFIX_LEVEL=2 )
add_definitions( -D LOGGER_ENABLE_COLORS=1 )
add_definitions( -D LOGGER_ENABLE_COLORS_ON_USER_HEADER=0 )

