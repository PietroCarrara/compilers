#include "intermediary-code.h"

#include <stdlib.h>

IntermediaryCode* make_code(IntermediaryCodeInstruction instruction) {
  IntermediaryCode* code = malloc(sizeof(IntermediaryCode));
  code->instruction = instruction;
  code->next = NULL;
  return code;
}

IntermediaryCode* append_code(IntermediaryCode* code, IntermediaryCode* appending) {
  if (code == NULL) {
    return appending;
  }
  append_code(code->next, appending);
  return code;
}

IntermediaryCode* make_declarations(DeclarationList* declarations) {
  IntermediaryCode* result = NULL;
  while (declarations != NULL) {
    IntermediaryCode* code = make_code(CodeDeclaration(declarations->declaration));
    append_code(result, code);

    declarations = declarations->next;
  }

  return result;
}

IntermediaryCode* make_intermediary_code(Program program) {
  IntermediaryCode* result = NULL;
  append_code(result, make_declarations(program.declarations));

  return result;
}
