#include "format.h"
#include "intermediary-code.h"
#include "semantic-check.h"
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
  if (argc < 2) {
    return 1;
  }

  yyin = fopen(argv[1], "r");
  if (yyin == NULL) {
    printf("error: could not open input file \"%s\": %s", argv[1], strerror(errno));
    return 1;
  }

  // Parse and check for error
  if (yyparse()) {
    printf("error: invalid syntax at file %s:%d\n", argv[1], yylineno);
    return 3;
  }

  SemanticErrorList* list = verify_program(yyprogram);
  if (list != NULL) {
    while (list != NULL) {
      printf("erro: %s\n", list->error.message);
      list = list->next;
    }
    return 4;
  }

  IntermediaryCode* ic = make_intermediary_code(program);
  print_intermediary_code(stdout, ic);

  return 0;
}