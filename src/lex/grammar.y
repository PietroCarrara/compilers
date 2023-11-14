%{
  int yylex(void);
  int yyerror(char* s);
%}

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
%token TOKEN_IDENTIFIER
%token TOKEN_INT_LITERAL
%token TOKEN_FLOAT_LITERAL
%token TOKEN_CHAR_LITERAL
%token TOKEN_STRING_LITERAL
%token TOKEN_ERROR /* TODO: Delete */

%left '&' '|' '~'
%left '<' '>' TOKEN_LESS_EQUAL TOKEN_GREATER_EQUAL TOKEN_DOUBLE_EQUALS TOKEN_NOT_EQUALS
%left '+' '-'
%left '*' '/'

%%

program: declarations implementations

declarations: declaration declarations
            |
            ;

declaration: variable_declaration
           | function_declaration
           | array_declaration
           ;

variable_declaration: type TOKEN_IDENTIFIER '=' literal ';'
                    ;

function_declaration: type TOKEN_IDENTIFIER '(' parameter_list ')' ';'
                    ;

array_declaration: type TOKEN_IDENTIFIER '[' TOKEN_INT_LITERAL ']' ';'
                 | type TOKEN_IDENTIFIER '[' TOKEN_INT_LITERAL ']' array_item_list ';'
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
          ;

argument_list:
             | non_empty_argument_list
             ;

non_empty_argument_list: expression ',' non_empty_argument_list
                       | expression
                       ;

type: TOKEN_CHAR
    | TOKEN_FLOAT
    | TOKEN_INT
    ;

literal: TOKEN_INT_LITERAL
       | TOKEN_FLOAT_LITERAL
       | TOKEN_CHAR_LITERAL
       | TOKEN_STRING_LITERAL
       ;

parameter_list: type TOKEN_IDENTIFIER ',' parameter_list
              | type TOKEN_IDENTIFIER
              |
              ;

array_item_list: literal array_item_list
               | literal
               ;