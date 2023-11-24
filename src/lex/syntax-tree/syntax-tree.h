#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include <datatype99.h>

typedef char* Identifier;

datatype(Type, (IntegerType), (FloatType), (CharType));

datatype(Literal, (IntLiteral, int), (FloatLiteral, float), (CharLiteral, char), (StringLiteral, char*));

typedef struct ParametersDeclaration {
  Type type;
  Identifier name;
  struct ParametersDeclaration* next;
} ParametersDeclaration;

typedef struct ArrayInitialization {
  Literal value;
  struct ArrayInitialization* next;
} ArrayInitialization;

datatype(
    Declaration, (VariableDeclaration, Type, Identifier, Literal),
    (FunctionDeclaration, Type, Identifier, ParametersDeclaration*),
    (ArrayDeclaration, Type, Identifier, int, ArrayInitialization*)
);

typedef struct DeclarationList {
  Declaration declaration;
  struct DeclarationList* next;
} DeclarationList;

datatype(
    BinaryOperator, (SumOperator), (SubtractionOperator), (MultiplicationOperator), (DivisionOperator),
    (LessThanOperator), (GreaterThanOperator), (AndOperator), (OrOperator), (NotOperator), (LessOrEqualOperator),
    (GreaterOrEqualOperator), (EqualsOperator), (DiffersOperator)
);

datatype(
    Expression, (LiteralExpression, Literal), (IdentifierExpression, Identifier),
    (ReadArrayExpression, Identifier, Expression*), (FunctionCallExpression, Identifier, struct ArgumentList*),
    (InputExpression, Type), (BinaryExpression, BinaryOperator, Expression*, Expression*)
);

typedef struct ArgumentList {
  Expression argument;
  struct ArgumentList* next;
} ArgumentList;

datatype(
    Statement, (AssignmentStatement, Identifier, Expression),
    (ArrayAssignmentStatement, Identifier, Expression, Expression), // array name, index expression, value expression
    (PrintStatement, Expression), (ReturnStatement, Expression),
    (IfStatement, Expression, Statement*),                 // condition, true block
    (IfElseStatement, Expression, Statement*, Statement*), // condition, true block, false block
    (WhileStatement, Expression, Statement*),              // condition, body
    (BlockStatement, struct StatementList*),               // body
    (EmptyStatement)
);

typedef struct StatementList {
  Statement statement;
  struct StatementList* next;
} StatementList;

typedef struct Implementation {
  Identifier name;
  Statement body;
} Implementation;


typedef struct ImplementationList {
  Implementation implementation;
  struct ImplementationList* next;
} ImplementationList;

typedef struct Program {
  DeclarationList* declarations;
  ImplementationList* implementations;
} Program;

DeclarationList* make_declaration(Declaration declaration);
ParametersDeclaration* make_parameters_declaration(Type type, Identifier name);
ArrayInitialization* make_array_initialization(Literal value);
StatementList* make_statement_list(Statement statement);
ArgumentList* make_argument_list(Expression argument);
ImplementationList* make_implementation_list(Implementation implementation);


Statement* make_statement(Statement statement);
Expression* make_expression(Expression expression);

#endif