#include "syntax-tree.h"

#include <stdlib.h>

DeclarationList* make_declaration(Declaration declaration) {
  DeclarationList* dec = malloc(sizeof(DeclarationList));
  dec->declaration = declaration;
  dec->next = NULL;
  return dec;
}

ParametersDeclaration* make_parameters_declaration(Type type, Identifier name) {
  ParametersDeclaration* dec = malloc(sizeof(ParametersDeclaration));
  dec->type = type;
  dec->name = name;
  dec->next = NULL;
  return dec;
}

ArrayInitialization* make_array_initialization(Literal value) {
  ArrayInitialization* init = malloc(sizeof(ArrayInitialization));
  init->value = value;
  init->next = NULL;
  return init;
}

StatementList* make_statement_list(Statement statement) {
  StatementList* init = malloc(sizeof(StatementList));
  init->statement = statement;
  init->next = NULL;
  return init;
}

ArgumentList* make_argument_list(Expression argument) {
  ArgumentList* list = malloc(sizeof(ArgumentList));
  list->argument = argument;
  list->next = NULL;
  return list;
}

ImplementationList* make_implementation_list(Implementation implementation) {
  ImplementationList* list = malloc(sizeof(ImplementationList));
  list->implementation = implementation;
  list->next = NULL;
  return list;
}


Expression* make_expression(Expression expression) {
  Expression* alloc = malloc(sizeof(Expression));
  *alloc = expression;
  return alloc;
}

Statement* make_statement(Statement statement) {
  Statement* alloc = malloc(sizeof(Statement));
  *alloc = statement;
  return alloc;
}