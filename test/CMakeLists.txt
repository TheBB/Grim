add_executable(grimtest main.c immediate-objects.c)
target_link_libraries(grimtest munit libgrim)

add_test(NAME immediate-objects COMMAND grimtest /immediate-objects)
