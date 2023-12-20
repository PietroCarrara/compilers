#include "semantic-check.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SemanticErrorList* make_semantic_error_list(SemanticError error) {
  SemanticErrorList* list = malloc(sizeof(SemanticErrorList));

  list->error = error;
  list->next = NULL;

  return list;
}

datatype(DeclarationSearchResult, (DeclarationNotFound), (DeclarationFound, Declaration));

DeclarationSearchResult find_declaration(Identifier target, DeclarationList* declarations) {
  if (declarations == NULL) {
    return DeclarationNotFound();
  }

  Identifier identifier;
  match(declarations->declaration) {
    of(VariableDeclaration, _, i) identifier = *i;
    of(FunctionDeclaration, _, i) identifier = *i;
    of(ArrayDeclaration, _, i) identifier = *i;
  }

  if (strcmp(identifier, target) == 0) {
    return DeclarationFound(declarations->declaration);
  }

  return find_declaration(target, declarations->next);
}

SemanticErrorList* verify_double_declarations(DeclarationList* declarations) {
  if (declarations == NULL) {
    return NULL;
  }

  Declaration declaration = declarations->declaration;

  Identifier identifier;
  match(declaration) {
    of(VariableDeclaration, _, i) identifier = *i;
    of(FunctionDeclaration, _, i) identifier = *i;
    of(ArrayDeclaration, _, i) identifier = *i;
  }

  SemanticErrorList* error = NULL;
  SemanticErrorList* next_errors = verify_double_declarations(declarations->next);

  if (MATCHES(find_declaration(identifier, declarations->next), DeclarationFound)) {
    char error_message[999];
    snprintf(error_message, sizeof(error_message), "identificador \"%s\" declarado mais de uma vez", identifier);

    error = make_semantic_error_list((SemanticError) { .message = strdup(error_message) });
    error->next = next_errors;
  }

  return error != NULL ? error : next_errors;
}

SemanticErrorList* verify_program(Program program) { return verify_double_declarations(program.declarations); }