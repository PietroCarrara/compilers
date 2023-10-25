#include <stdio.h>

extern int yylex();
extern char* yytext;
extern FILE* yyin;

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
}