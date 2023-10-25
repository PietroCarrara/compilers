find_package(FLEX)
FLEX_TARGET(lex lex.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)

add_library(lex ${FLEX_lex_OUTPUTS} tokenization.h tokens.h)
target_include_directories(lex PUBLIC ./)