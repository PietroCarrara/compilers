#include "symbols.h"

#include <stdio.h>

extern int yylex();
extern char* yytext;
extern FILE* yyin;

static void print_symbol(SymbolData* data) {
  match(data->symbol) {
    of(IntLiteral, i) printf("int(%d)", *i);
    of(FloatLiteral, f) printf("float(%f)", *f);
    of(CharLiteral, c) printf("char(%c)", *c);
    of(StringLiteral, s) printf("string(%s)", *s);
    of(IdentifierLiteral, n) printf("identifier(%s)", *n);
  }
  printf("\n");
}

int main(int argc, char** argv) {
  if (argc < 2) {
    return 1;
  }

  yyin = fopen(argv[1], "r");

  int token = yylex();
  while (token != 0) {
    token = yylex();
  }
  printf("EOF\n");

  foreach_symbol(print_symbol);
}