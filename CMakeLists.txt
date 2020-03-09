cmake_minimum_required(VERSION 3.5)
enable_language(C)
project(grim)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

enable_testing()

add_subdirectory("${CMAKE_SOURCE_DIR}/vendor")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/libgrim")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/grim")
add_subdirectory("${CMAKE_SOURCE_DIR}/test")

target_compile_options(libgrim PRIVATE -Wall -Wextra)
target_compile_options(grim PRIVATE -Wall -Wextra)
