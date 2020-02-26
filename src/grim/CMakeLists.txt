add_executable(grim main.c)
target_link_libraries(grim libgrim)
set_property(TARGET grim PROPERTY C_STANDARD 11)
