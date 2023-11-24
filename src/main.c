#include <errno.h>
#include <stdio.h>
#include <string.h>

extern int yyparse(void);
extern FILE* yyin;
extern int yylineno;

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

  return 0;
}
