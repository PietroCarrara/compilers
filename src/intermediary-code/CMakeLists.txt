add_library(intermediary-code intermediary-code.c print-intermediary-code.c intermediary-code.h)
target_include_directories(intermediary-code INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(intermediary-code syntax-tree)