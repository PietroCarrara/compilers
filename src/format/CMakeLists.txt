add_library(format format.c format.h)
target_include_directories(format INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(format syntax-tree)