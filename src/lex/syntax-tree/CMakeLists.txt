add_library(syntax-tree syntax-tree.h syntax-tree.c)
target_include_directories(syntax-tree INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(syntax-tree datatype99)
