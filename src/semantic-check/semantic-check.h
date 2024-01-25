#ifndef SEMANTIC_CHECK_H

#include "syntax-tree.h"

typedef struct {
  char* message;
} SemanticError;

typedef struct SemanticErrorList {
  SemanticError error;
  struct SemanticErrorList* next;
} SemanticErrorList;

SemanticErrorList* verify_program(Program Program);

datatype(DeclarationSearchResult, (DeclarationNotFound), (DeclarationFound, Declaration, Type, Identifier));
DeclarationSearchResult find_declaration(Identifier target, DeclarationList* declarations);

#endif