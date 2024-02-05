add_library(asm asm.c asm.h)
target_include_directories(asm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(asm intermediary-code)