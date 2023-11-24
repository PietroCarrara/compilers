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
