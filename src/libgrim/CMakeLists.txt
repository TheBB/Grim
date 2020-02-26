find_package(GMP REQUIRED)

add_library(libgrim SHARED grim.c)
set_target_properties(libgrim PROPERTIES
  C_STANDARD 11
  OUTPUT_NAME grim
)
target_include_directories(libgrim INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(libgrim PRIVATE "${CMAKE_SOURCE_DIR}/vendor/gc/include")
target_include_directories(libgrim PRIVATE ${GMP_INCLUDES})
target_link_libraries(libgrim gc-lib ${GMP_LIBRARIES})
