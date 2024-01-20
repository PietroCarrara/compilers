cmake_minimum_required(VERSION 3.0.0)
project(compilerProject VERSION 0.1.0 LANGUAGES C)

include(CTest)
include(FetchContent)

enable_testing()

FetchContent_Declare(
    datatype99
    URL https://github.com/Hirrolot/datatype99/archive/refs/tags/v1.6.4.tar.gz
)
FetchContent_MakeAvailable(datatype99)

add_executable(compilerProject src/main.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Disable full macro expansion backtraces for Metalang99.
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  target_compile_options(compilerProject PRIVATE -fmacro-backtrace-limit=1)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  target_compile_options(compilerProject PRIVATE -ftrack-macro-expansion=0)
endif()

add_subdirectory(src)

target_link_libraries(compilerProject datatype99 lex yacc syntax-tree format semantic-check intermediary-code)
