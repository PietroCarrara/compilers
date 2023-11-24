%{
  #include <stdlib.h>
  #include "syntax-tree.h"

  int yylex(void);
  int yyerror(char* s);
%}

%union {
       Program program;
       DeclarationList* declarationList;
       ImplementationList* implementationList;
       Declaration declaration;
       ParametersDeclaration* parametersDeclaration;
       ArrayInitialization* arrayInitialization;
       Type type;
       Identifier identifier;
       Literal literal;

       int int_val;
       float float_val;
       char char_val;
       char* string_val;
};

%token TOKEN_CHAR
%token TOKEN_INT
%token TOKEN_FLOAT
%token TOKEN_CODE
%token TOKEN_IF
%token TOKEN_ELSE
%token TOKEN_WHILE
%token TOKEN_INPUT
%token TOKEN_PRINT
%token TOKEN_RETURN
%token TOKEN_LESS_EQUAL
%token TOKEN_GREATER_EQUAL
%token TOKEN_DOUBLE_EQUALS
%token TOKEN_NOT_EQUALS
%token <identifier>TOKEN_IDENTIFIER
%token <int_val>TOKEN_INT_LITERAL
%token <float_val>TOKEN_FLOAT_LITERAL
%token <char_val>TOKEN_CHAR_LITERAL
%token <string_val>TOKEN_STRING_LITERAL

%left '&' '|' '~'
%left '<' '>' TOKEN_LESS_EQUAL TOKEN_GREATER_EQUAL TOKEN_DOUBLE_EQUALS TOKEN_NOT_EQUALS
%left '+' '-'
%left '*' '/'

%type <program> program

%type <declarationList> declarations
%type <declaration> declaration
%type <declaration> variable_declaration
%type <declaration> function_declaration
%type <declaration> array_declaration

%type <type> type
%type <literal> literal

%type <parametersDeclaration> parameters_declaration
%type <parametersDeclaration> non_empty_parameters_declaration
%type <arrayInitialization> array_item_list

%type <implementationList> implementations

%%

program: declarations implementations { $$ = (Program){ .declarations = $1, .implementations = $2 }; }

declarations: declaration declarations { $$ = make_declaration($1); $$->next = $2; }
            |                          { $$ = NULL; }
            ;

declaration: variable_declaration
           | function_declaration
           | array_declaration
           ;

variable_declaration: type TOKEN_IDENTIFIER '=' literal ';' { $$ = VariableDeclaration($1, $2, $4); }
                    ;

function_declaration: type TOKEN_IDENTIFIER '(' parameters_declaration ')' ';' { $$ = FunctionDeclaration($1, $2, $4); }
                    ;

array_declaration: type TOKEN_IDENTIFIER '[' TOKEN_INT_LITERAL ']' ';'                 { $$ = ArrayDeclaration($1, $2, $4, NULL); }
                 | type TOKEN_IDENTIFIER '[' TOKEN_INT_LITERAL ']' array_item_list ';' { $$ = ArrayDeclaration($1, $2, $4, $6); }
                 ;

implementations: implementation implementations
               |
               ;

implementation: TOKEN_CODE TOKEN_IDENTIFIER command

command: TOKEN_IDENTIFIER '=' expression ';'
       | TOKEN_IDENTIFIER '[' expression ']' '=' expression ';'
       | TOKEN_PRINT expression ';'
       | TOKEN_RETURN expression ';'
       | TOKEN_IF '(' expression ')' command
       | TOKEN_IF '(' expression ')' command TOKEN_ELSE command
       | TOKEN_WHILE '(' expression ')' command
       | '{' command_sequence '}'
       | ';' /* Empty command */
       ;

command_sequence: command command_sequence
                |
                ;

expression: literal
          | TOKEN_IDENTIFIER
          | TOKEN_IDENTIFIER '[' expression ']'
          | TOKEN_IDENTIFIER '(' argument_list ')'
          | TOKEN_INPUT '(' type ')'
          | expression '+' expression
          | expression '-' expression
          | expression '*' expression
          | expression '/' expression
          | expression '<' expression
          | expression '>' expression
          | expression '&' expression
          | expression '|' expression
          | expression '~' expression
          | expression TOKEN_LESS_EQUAL expression
          | expression TOKEN_GREATER_EQUAL expression
          | expression TOKEN_DOUBLE_EQUALS expression
          | expression TOKEN_NOT_EQUALS expression
          | '(' expression ')'
          ;

argument_list: non_empty_argument_list
             |
             ;

non_empty_argument_list: expression ',' non_empty_argument_list
                       | expression
                       ;

type: TOKEN_CHAR  { $$ = CharType(); }
    | TOKEN_FLOAT { $$ = FloatType(); }
    | TOKEN_INT   { $$ = IntegerType(); }
    ;

literal: TOKEN_INT_LITERAL    { $$ = IntLiteral($1); }
       | TOKEN_FLOAT_LITERAL  { $$ = FloatLiteral($1); }
       | TOKEN_CHAR_LITERAL   { $$ = CharLiteral($1); }
       | TOKEN_STRING_LITERAL { $$ = StringLiteral($1); }
       ;

parameters_declaration: non_empty_parameters_declaration
              |                                          { $$ = NULL; }
              ;

non_empty_parameters_declaration: type TOKEN_IDENTIFIER ',' non_empty_parameters_declaration { $$ = make_parameters_declaration($1, $2); $$->next = $4; }
                                | type TOKEN_IDENTIFIER                                      { $$ = NULL; }
                                ;

array_item_list: literal array_item_list { $$ = make_array_initialization($1); $$->next = $2; }
               | literal                 { $$ = make_array_initialization($1); }
               ;
