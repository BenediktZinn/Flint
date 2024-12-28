cmake_minimum_required(VERSION 3.16)
project(Flint)

set(CMAKE_CXX_STANDARD 17)

add_executable(Flint src/main.cpp)

target_link_libraries(Flint
        "-framework IOKit"
        "-framework CoreFoundation"
)
