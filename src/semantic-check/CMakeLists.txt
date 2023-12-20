add_library(semantic-check semantic-check.c semantic-check.h)
target_include_directories(semantic-check INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(semantic-check syntax-tree)