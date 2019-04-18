link_directories(..)
include_directories(..)
add_executable(demo1 demo1.c)
target_link_libraries(demo1 spool)
#target_include_directories(demo1 spool)

