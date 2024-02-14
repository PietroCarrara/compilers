%{
  #include <stdlib.h>
  #include "syntax-tree.h"

  int yylex(void);
  int yyerror(char* s);

  extern Program yyprogram;
%}

%union {
       Program program;

       DeclarationList* declarationList;
       Declaration declaration;
       ParametersDeclaration* parametersDeclaration;
       ArrayInitialization* arrayInitialization;

       Implementation implementation;
       ImplementationList* implementationList;

       Statement statement;
       StatementList *statementList;

       Expression expression;
       ArgumentList* argumentList;

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
%type <implementation> implementation

%type <statement> command;
%type <statementList> command_sequence;

%type <expression> expression;

%type <argumentList> argument_list
%type <argumentList> non_empty_argument_list

%%

program: declarations implementations { $$ = (Program){ .declarations = $1, .implementations = $2 }; yyprogram = $$; }

declarations: declaration declarations { $$ = make_declaration($1); $$->next = $2; }
            | error ';' declarations   { $$ = $3; }
            | declarations ';' error   { $$ = $1; }
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

implementations: implementation implementations { $$ = make_implementation_list($1); $$->next = $2; }
               | error implementations          { $$ = $2; }
               | implementations error          { $$ = $1; }
               |                                { $$ = NULL; }
               ;

implementation: TOKEN_CODE TOKEN_IDENTIFIER command { $$ = (Implementation){ .name = $2, .body = $3 }; }

command: TOKEN_IDENTIFIER '=' expression ';'                    { $$ = AssignmentStatement($1, $3); }
       | TOKEN_IDENTIFIER '[' expression ']' '=' expression ';' { $$ = ArrayAssignmentStatement($1, $3, $6); }
       | TOKEN_PRINT expression ';'                             { $$ = PrintStatement($2); }
       | TOKEN_RETURN expression ';'                            { $$ = ReturnStatement($2); }
       | TOKEN_IF '(' expression ')' command                    { $$ = IfStatement($3, make_statement($5)); }
       | TOKEN_IF '(' expression ')' command TOKEN_ELSE command { $$ = IfElseStatement($3, make_statement($5), make_statement($7)); }
       | TOKEN_WHILE '(' expression ')' command                 { $$ = WhileStatement($3, make_statement($5)); }
       | '{' command_sequence '}'                               { $$ = BlockStatement($2); }
       | ';' /* Empty command */                                { $$ = EmptyStatement(); }
       | command error ';'                                      { $$ = $1; }
       ;

command_sequence: command command_sequence { $$ = make_statement_list($1); $$->next = $2; }
                |                          { $$ = NULL; }
                ;

expression: literal                                   { $$ = LiteralExpression($1); }
          | TOKEN_IDENTIFIER                          { $$ = IdentifierExpression($1); }
          | TOKEN_IDENTIFIER '[' expression ']'       { $$ = ReadArrayExpression($1, make_expression($3)); }
          | TOKEN_IDENTIFIER '(' argument_list ')'    { $$ = FunctionCallExpression($1, $3); }
          | TOKEN_INPUT '(' type ')'                  { $$ = InputExpression($3); }
          | expression '+' expression                 { $$ = BinaryExpression(SumOperator(), make_expression($1), make_expression($3)); }
          | expression '-' expression                 { $$ = BinaryExpression(SubtractionOperator(), make_expression($1), make_expression($3)); }
          | expression '*' expression                 { $$ = BinaryExpression(MultiplicationOperator(), make_expression($1), make_expression($3)); }
          | expression '/' expression                 { $$ = BinaryExpression(DivisionOperator(), make_expression($1), make_expression($3)); }
          | expression '<' expression                 { $$ = BinaryExpression(LessThanOperator(), make_expression($1), make_expression($3)); }
          | expression '>' expression                 { $$ = BinaryExpression(GreaterThanOperator(), make_expression($1), make_expression($3)); }
          | expression '&' expression                 { $$ = BinaryExpression(AndOperator(), make_expression($1), make_expression($3)); }
          | expression '|' expression                 { $$ = BinaryExpression(OrOperator(), make_expression($1), make_expression($3)); }
          | expression '~' expression                 { $$ = BinaryExpression(NotOperator(), make_expression($1), make_expression($3)); }
          | expression TOKEN_LESS_EQUAL expression    { $$ = BinaryExpression(LessOrEqualOperator(), make_expression($1), make_expression($3)); }
          | expression TOKEN_GREATER_EQUAL expression { $$ = BinaryExpression(GreaterOrEqualOperator(), make_expression($1), make_expression($3)); }
          | expression TOKEN_DOUBLE_EQUALS expression { $$ = BinaryExpression(EqualsOperator(), make_expression($1), make_expression($3)); }
          | expression TOKEN_NOT_EQUALS expression    { $$ = BinaryExpression(DiffersOperator(), make_expression($1), make_expression($3)); }
          | '(' expression ')'                        { $$ = $2; }
          ;

argument_list: non_empty_argument_list
             |                          { $$ = NULL; }
             ;

non_empty_argument_list: expression ',' non_empty_argument_list { $$ = make_argument_list($1); $$->next = $3; }
                       | expression                             { $$ = make_argument_list($1); }
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
                                | type TOKEN_IDENTIFIER                                      { $$ = make_parameters_declaration($1, $2); }
                                ;

array_item_list: literal array_item_list { $$ = make_array_initialization($1); $$->next = $2; }
               | literal                 { $$ = make_array_initialization($1); }
               ;
