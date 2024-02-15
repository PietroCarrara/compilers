#include "asm.h"
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
extern int has_error;

Program yyprogram;

int main(int argc, char** argv) {
  if (argc < 2) {
    return 1;
  }

  yyin = fopen(argv[1], "r");
  if (yyin == NULL) {
    fprintf(stderr, "error: could not open input file \"%s\": %s", argv[1], strerror(errno));
    return 1;
  }

  // Parse and check for error
  if (yyparse()) {
    return 3;
  }

  SemanticErrorList* list = verify_program(yyprogram);
  if (list != NULL) {
    while (list != NULL) {
      fprintf(stderr, "warning: %s\n", list->error.message);
      list = list->next;
    }
    // return 4;
  }

  if (has_error != 0) {
    return 3;
  }

  FILE* out = fopen("out.s", "w+");
  write_asm(yyprogram, out);

  return 0;
}