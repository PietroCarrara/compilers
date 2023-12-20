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

#endif