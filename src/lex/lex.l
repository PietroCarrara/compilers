%{
  #include "tokenization.h"
%}

%option noyywrap yylineno

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

[a-zA-Z_0-9]*[a-zA-Z_]+[a-zA-Z_0-9]* { return TOKEN_IDENTIFIER; }


%{/* String literal */%}
\"[^"]*\" { return TOKEN_STRING_LITERAL; }

[1-9][0-9]*          { return TOKEN_INT_LITERAL; }
[1-9][0-9]*\.[0-9]+  { return TOKEN_FLOAT_LITERAL; }
'.'                  { return TOKEN_CHAR_LITERAL; }


"//"[^/].*             { /* single-line comment */ }
"///"                  { BEGIN(MULTICOMMENT); }
<MULTICOMMENT>"\\\\\\" { BEGIN(INITIAL); }
<MULTICOMMENT>"\n"     ;
<MULTICOMMENT>.        ;

[ \t\n] { /* ignore whitespaces */ }
.       { return TOKEN_ERROR; }

