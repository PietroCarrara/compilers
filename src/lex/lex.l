%{
  #include "tokenization.h"
%}

%option noyywrap

%x MULTICOMMENT

%%

"char"   { return TOKEN_CHAR; }
"int"    { return TOKEN_INT; }
"real"   { return TOKEN_REAL; }
"bool"   { return TOKEN_BOOL; }
"if"     { return TOKEN_IF; }
"then"   { return TOKEN_THEN; }
"else"   { return TOKEN_ELSE; }
"loop"   { return TOKEN_LOOP; }
"input"  { return TOKEN_INPUT; }
"output" { return TOKEN_OUTPUT; }
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

%{/* String literal accounting for \" inside itself */%}
\"("\\\""|[^"\n])*\" { return TOKEN_STRING_LITERAL; }

[1-9][0-9]          { return TOKEN_INT_LITERAL; }
[1-9][0-9]*\.[0-9]+ { return TOKEN_REAL_LITERAL; }
'.'                 { return TOKEN_CHAR_LITERAL; }

[a-zA-Z._]+          { return TOKEN_IDENTIFIER; }

"\\\\"[^\\].*       ;
"\\\\\\"            { BEGIN(MULTICOMMENT); }
<MULTICOMMENT>"///" { BEGIN(INITIAL); }
<MULTICOMMENT>"\n"  ;
<MULTICOMMENT>.     ;

[ \t\n] ;
