%{
  #include "symbols.h"
  #include "y.tab.h"

  #include <string.h>

  // Code required by the professor
  // TODO: Delete
  void initMe() { }
  int _running = 1;
  int yywrap() { _running = 0; return 1; }
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

[a-zA-Z_0-9]*[a-zA-Z_]+[a-zA-Z_0-9]* {
  register_symbol((SymbolData){
    .name = strdup(yytext),
    .symbol = IdentifierLiteral(strdup(yytext))
  });
  return TOKEN_IDENTIFIER;
}
%{/* String literal accounting for \" inside itself */%}
\"("\\\""|[^"\n])*\" {
  register_symbol((SymbolData){
    .name = strdup(yytext),
    .symbol = StringLiteral(strdup(yytext))
  });
  return TOKEN_STRING_LITERAL;
}
[0-9]+ {
  register_symbol((SymbolData){
    .name = strdup(yytext),
    .symbol = IntLiteral(atoi(yytext))
  });
  return TOKEN_INT_LITERAL;
}
[0-9]+\.[0-9]+ {
  register_symbol((SymbolData){
    .name = strdup(yytext),
    .symbol = FloatLiteral(atof(yytext))
  });
  return TOKEN_FLOAT_LITERAL;
}
'.' {
  register_symbol((SymbolData){
    .name = strdup(yytext),
    .symbol = CharLiteral(yytext[1])
  });
  return TOKEN_CHAR_LITERAL;
}

"//"[^/].*             { /* single-line comment */ }
"///"                  { BEGIN(MULTICOMMENT); }
<MULTICOMMENT>"\\\\\\" { BEGIN(INITIAL); }
<MULTICOMMENT>"\n"     ;
<MULTICOMMENT>.        ;

[ \t\n] { /* ignore whitespaces */ }
.       { printf("error: unknown token \"%s\"\n", yytext); return TOKEN_ERROR; }

%%

void yyerror(char const *s) {
  fprintf(stderr, "error: %s\n", s);
}