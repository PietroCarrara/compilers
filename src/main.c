#include <stdio.h>

extern int yylex();
extern char* yytext;
extern FILE* yyin;

int main() {
  yyin = fopen("input.lang", "r");

  int token = yylex();
  while (token != 0) {
    token = yylex();
  }
  printf("EOF\n");
}