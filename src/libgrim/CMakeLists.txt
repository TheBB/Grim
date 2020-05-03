find_package(GMP REQUIRED)
find_package(Unistring REQUIRED)

add_library(libgrim SHARED
  grim.c objects.c strings.c numbers.c
  funcs.c hashing.c parsing.c modules.c
  exec.c builtins.c
)
set_target_properties(libgrim PROPERTIES
  C_STANDARD 11
  OUTPUT_NAME grim
)
target_include_directories(libgrim INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(libgrim PRIVATE "${CMAKE_SOURCE_DIR}/vendor/gc/include")
target_include_directories(libgrim PRIVATE ${GMP_INCLUDES})
target_include_directories(libgrim PRIVATE ${UNISTRING_INCLUDE_DIR})
target_link_libraries(libgrim gc-lib murmur)
target_link_libraries(libgrim ${GMP_LIBRARIES})
target_link_libraries(libgrim ${UNISTRING_LIBRARY})
