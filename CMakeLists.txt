cmake_minimum_required(VERSION 3.15)
project(SerialKit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SERIALKIT_BUILD_TESTS "Build tests" ON)
option(SERIALKIT_BUILD_COMPILER "Build compiler" ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(Threads REQUIRED)

add_subdirectory(runtime)

if(SERIALKIT_BUILD_COMPILER)
    add_subdirectory(compiler)
endif()

if(SERIALKIT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
