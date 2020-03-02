add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/gc")

add_library(munit SHARED "${CMAKE_CURRENT_SOURCE_DIR}/munit/munit.c")
target_include_directories(munit INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/munit")

add_library(zhash STATIC "${CMAKE_CURRENT_SOURCE_DIR}/zhash-c/src/zhash.c")
target_include_directories(zhash INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/zhash-c/src")
