find_package(FLEX)
FLEX_TARGET(lex lex.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)

add_subdirectory(symbols)

add_library(lex ${FLEX_lex_OUTPUTS})
target_include_directories(lex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/symbols)
target_link_libraries(lex symbols)
