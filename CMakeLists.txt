cmake_minimum_required(VERSION 2.8.3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/googletest/googletest)

project(nbbt)

include_directories(include)

file(GLOB lib_src "src/*.cpp")
file(GLOB lib_h "include/nbbt/*.h")
add_library(${PROJECT_NAME} STATIC ${lib_src} ${lib_h})

add_subdirectory(test)
