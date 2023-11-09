#include "symbols.h"

#include <stdio.h>

extern int yyparse(void);
extern FILE* yyin;
extern int yylineno;

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

  // Check for error
  if (yyparse()) {
    printf("error: invalid syntax at file %s:%d\n", argv[1], yylineno);
  }

  foreach_symbol(print_symbol);
}