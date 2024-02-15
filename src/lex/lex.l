%{
  #include "syntax-tree.h"
  #include "y.tab.h"

  #include <string.h>
  #include <stdlib.h>

  int has_error = 0;

  int yywrap(void) {
    return 1;
  }

  char* trim_quotes(char* str) {
    int size = strlen(str);
    int allocated = (size-2+1); // Trim two quotes, reserve space for \0
    char* res = malloc(allocated*sizeof(char));
    memcpy(res, &str[1], size-2);
    res[allocated-1] = 0;
    return res;
  }
%}

%option yylineno

%x MULTICOMMENT

%%

"char"   { return TOKEN_CHAR; }
"int"    { return TOKEN_INT; }
"float"  { return TOKEN_FLOAT; }
"code"   { return TOKEN_CODE; }
"if"     { return TOKEN_IF; }
"else"   { return TOKEN_ELSE; }
"while"  { return TOKEN_WHILE; }
"input"  { return TOKEN_INPUT; }
"print"  { return TOKEN_PRINT; }
"return" { return TOKEN_RETURN; }

[-+*/%,;()\[\]{}=<>&|~] { return yytext[0]; }

"<=" { return TOKEN_LESS_EQUAL; }
">=" { return TOKEN_GREATER_EQUAL; }
"==" { return TOKEN_DOUBLE_EQUALS; }
"!=" { return TOKEN_NOT_EQUALS; }

[a-zA-Z_0-9]*[a-zA-Z_]+[a-zA-Z_0-9]* { yylval.identifier = strdup(yytext); return TOKEN_IDENTIFIER; }
\"("\\\""|[^"\n])*\"                 { yylval.string_val = trim_quotes(yytext); return TOKEN_STRING_LITERAL; }
[0-9]+                             { yylval.int_val = atoi(yytext); return TOKEN_INT_LITERAL; }
[0-9]+\.[0-9]+                       { yylval.float_val = atof(yytext); return TOKEN_FLOAT_LITERAL; }
'.'                                  { yylval.char_val = yytext[1]; return TOKEN_CHAR_LITERAL; }

"//"[^/].*             { /* single-line comment */ }
"///"                  { BEGIN(MULTICOMMENT); }
<MULTICOMMENT>"\\\\\\" { BEGIN(INITIAL); }
<MULTICOMMENT>"\n"     ;
<MULTICOMMENT>.        ;

[ \t\n] { /* ignore whitespaces */ }

%%

void yyerror(char const *s) {
  has_error = 1;
  /* fprintf(stderr, "error at line %d\n", yylineno); */
}