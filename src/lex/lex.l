%{
  #include "symbols.h"
  #include "tokenization.h"

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

"," { return TOKEN_COMMA; }
";" { return TOKEN_SEMICOLON; }
"(" { return TOKEN_OPEN_PAREN; }
")" { return TOKEN_CLOSE_PAREN; }
"[" { return TOKEN_OPEN_BRACKET; }
"]" { return TOKEN_CLOSE_BRACKET; }
"{" { return TOKEN_OPEN_BRACE; }
"}" { return TOKEN_CLOSE_BRACE; }
"=" { return TOKEN_EQUAL; }
"+" { return TOKEN_PLUS; }
"-" { return TOKEN_MINUS; }
"*" { return TOKEN_ASTERISK; }
"/" { return TOKEN_SLASH; }
"%" { return TOKEN_PERCENT; }
"<" { return TOKEN_LESS; }
">" { return TOKEN_GREATER; }
"&" { return TOKEN_AMPERSAND; }
"|" { return TOKEN_PIPE; }
"~" { return TOKEN_TILDE; }

[a-zA-Z_0-9]*[a-zA-Z_]+[a-zA-Z_0-9]* {
  register_symbol((SymbolData){
    .name = strdup(yytext),
    .symbol = IdentifierLiteral(strdup(yytext))
  });
  return TOKEN_IDENTIFIER;
}
\"[^"]*\" {
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
.       { return TOKEN_ERROR; }

