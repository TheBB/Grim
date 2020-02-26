add_executable(immediate-objects immediate-objects.c)
target_link_libraries(immediate-objects munit libgrim)
add_test(NAME immediate-objects COMMAND immediate-objects)
