#ifndef TOKENIZATION_H
#define TOKENIZATION_H

#include "tokens.h"

// Custom names so that I can control my own code and not depend on the naming
// conventions of the professor
#define TOKEN_CHAR   KW_CHAR
#define TOKEN_INT    KW_INT
#define TOKEN_REAL   KW_REAL
#define TOKEN_BOOL   KW_BOOL
#define TOKEN_IF     KW_IF
#define TOKEN_THEN   KW_THEN
#define TOKEN_ELSE   KW_ELSE
#define TOKEN_LOOP   KW_LOOP
#define TOKEN_INPUT  KW_INPUT
#define TOKEN_OUTPUT KW_OUTPUT
#define TOKEN_RETURN KW_RETURN

#define TOKEN_COMMA         ','
#define TOKEN_SEMICOLON     ';'
#define TOKEN_OPEN_PAREN    '('
#define TOKEN_CLOSE_PAREN   ')'
#define TOKEN_OPEN_BRACKET  '['
#define TOKEN_CLOSE_BRACKET ']'
#define TOKEN_OPEN_BRACE    '{'
#define TOKEN_CLOSE_BRACE   '}'
#define TOKEN_EQUAL         '='
#define TOKEN_PLUS          '+'
#define TOKEN_MINUS         '-'
#define TOKEN_ASTERISK      '*'
#define TOKEN_SLASH         '/'
#define TOKEN_PERCENT       '%'
#define TOKEN_LESS          '<'
#define TOKEN_GREATER       '>'
#define TOKEN_AMPERSAND     '&'
#define TOKEN_PIPE          '|'
#define TOKEN_TILDE         '~'

#define TOKEN_LESS_EQUAL    OPERATOR_LE
#define TOKEN_GREATER_EQUAL OPERATOR_GE
#define TOKEN_DOUBLE_EQUALS OPERATOR_EQ
#define TOKEN_NOT_EQUALS    OPERATOR_DIF

#define TOKEN_IDENTIFIER TK_IDENTIFIER

#define TOKEN_INT_LITERAL    LIT_INT
#define TOKEN_REAL_LITERAL   LIT_REAL
#define TOKEN_CHAR_LITERAL   LIT_CHAR
#define TOKEN_STRING_LITERAL LIT_STRING

#endif