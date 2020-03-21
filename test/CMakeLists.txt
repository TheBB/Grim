add_executable(
  grimtest
  main.c
  immediate-objects.c
  strings.c
)
target_link_libraries(grimtest munit libgrim)

add_test(NAME immediate-objects COMMAND grimtest /immediate-objects)
add_test(NAME strings COMMAND grimtest /strings)
