#ifndef INTERMEDIARY_CODE_H
#define INTERMEDIARY_CODE_H

#include "syntax-tree.h"

#include <datatype99.h>
#include <stdio.h>

typedef char* Label;

// TODO: Get rid of Jumps (and therefore labels). Use high level constructs, let the printing handle the labels.
// Maybe don't even have this? Just print directly from the AST?
datatype(
    IntermediaryCodeInstruction, (CodeDeclaration, Declaration), (CodeLoad, Identifier), (CodeStore, Identifier),
    (CodeReturnValue), (CodeReturn), (CodeRead, Type), (CodePrint), (CodeLoadArray, Identifier),
    (CodeStoreArray, Identifier), (CodeBinary, BinaryOperator), (CodeJump, Label*), (CodeJumpIfFalse, Label*),
    (CodeCall, Label*, ArgumentList*), (CodeNop)
);

typedef struct IntermediaryCode {
  Label* label;
  IntermediaryCodeInstruction instruction;
  struct IntermediaryCode* next;
} IntermediaryCode;

IntermediaryCode* make_intermediary_code(Program program);
void print_intermediary_code(FILE* out, IntermediaryCode* code);

#endif