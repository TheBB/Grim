add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/gc")

add_library(munit SHARED "${CMAKE_CURRENT_SOURCE_DIR}/munit/munit.c")
target_include_directories(munit INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/munit")

add_library(murmur STATIC "${CMAKE_CURRENT_SOURCE_DIR}/murmur/murmur.c")
target_include_directories(murmur INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/murmur")
