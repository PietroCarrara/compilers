find_package(FLEX)
find_package(BISON)
FLEX_TARGET(lex lex.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)
BISON_TARGET(yacc grammar.y ${CMAKE_CURRENT_BINARY_DIR}/y.tab.c DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/y.tab.h)

add_subdirectory(symbols)

add_library(lex ${FLEX_lex_OUTPUTS})
add_library(yacc ${BISON_yacc_OUTPUTS})
target_include_directories(lex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/symbols)
target_link_libraries(lex symbols yacc)
target_link_libraries(yacc lex)
