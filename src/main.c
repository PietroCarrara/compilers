#include "format.h"
#include "syntax-tree.h"
#include "y.tab.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

extern int yyparse(void);
extern FILE* yyin;
extern int yylineno;

Program yyprogram;

int main(int argc, char** argv) {
  if (argc < 3) {
    return 1;
  }

  yyin = fopen(argv[1], "r");
  if (yyin == NULL) {
    printf("error: could not open input file \"%s\": %s", argv[1], strerror(errno));
    return 1;
  }

  FILE* out = fopen(argv[2], "w");
  if (out == NULL) {
    printf("error: could not open output file \"%s\": %s", argv[2], strerror(errno));
    return 1;
  }

  // Parse and check for error
  if (yyparse()) {
    printf("error: invalid syntax at file %s:%d\n", argv[1], yylineno);
    return 3;
  }

  print_program(out, yyprogram);

  return 0;
}