%{
  #include "syntax-tree.h"
  #include "y.tab.h"

  #include <string.h>

  // Code required by the professor
  // TODO: Delete
  void initMe() { }
  int _running = 1;
  int yywrap(void) { _running = 0; return 1; }
  int isRunning() { return _running; }
  int getLineNumber() { return yylineno; }
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

[a-zA-Z_0-9]*[a-zA-Z_]+[a-zA-Z_0-9]* { return TOKEN_IDENTIFIER; }
\"("\\\""|[^"\n])*\"                 { return TOKEN_STRING_LITERAL; }
[0-9]+                               { return TOKEN_INT_LITERAL; }
[0-9]+\.[0-9]+                       { return TOKEN_FLOAT_LITERAL; }
'.'                                  { return TOKEN_CHAR_LITERAL; }

"//"[^/].*             { /* single-line comment */ }
"///"                  { BEGIN(MULTICOMMENT); }
<MULTICOMMENT>"\\\\\\" { BEGIN(INITIAL); }
<MULTICOMMENT>"\n"     ;
<MULTICOMMENT>.        ;

[ \t\n] { /* ignore whitespaces */ }

%%

void yyerror(char const *s) {
  fprintf(stderr, "error: %s\n", s);
}