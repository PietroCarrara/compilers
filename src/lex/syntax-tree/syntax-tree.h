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

typedef struct ImplementationList {
  struct ImplementationList* next;
} ImplementationList;

typedef struct StatementList {
  Statement statement;
  StatementList* next;
} StatementList;

typedef struct Program {
  DeclarationList* declarations;
  ImplementationList* implementations;
} Program;

DeclarationList* make_declaration(Declaration declaration);
ParametersDeclaration* make_parameters_declaration(Type type, Identifier name);
ArrayInitialization* make_array_initialization(Literal value);

#endif