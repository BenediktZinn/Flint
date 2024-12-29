cmake_minimum_required(VERSION 3.16)
project(Flint)

set(CMAKE_CXX_STANDARD 17)

add_executable(Flint src/main.cpp
        src/device-detection.cpp
        src/device-detection.h
        src/utils.h
        src/utils.cpp
        src/mtp-utils.cpp)

target_link_libraries(Flint
        "-framework IOKit"
        "-framework CoreFoundation"
)
