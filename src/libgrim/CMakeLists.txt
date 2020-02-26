add_library(libgrim SHARED grim.c)
target_include_directories(libgrim INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(libgrim PRIVATE "${CMAKE_SOURCE_DIR}/vendor/gc/include")
target_link_libraries(libgrim gc-lib)
