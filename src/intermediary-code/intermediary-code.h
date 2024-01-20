#ifndef INTERMEDIARY_CODE_H
#define INTERMEDIARY_CODE_H

#include "syntax-tree.h"

#include <datatype99.h>
#include <stdio.h>

typedef int Register;

datatype(
    IntermediaryCodeInstruction, (CodeDeclaration, Declaration), (CodeLoad, Identifier, Register),
    (CodeStore, Register, Identifier), (CodeBinary, Register, Register, BinaryOperator),
    (CodeJump, IntermediaryCodeInstruction*), (CodeJumpIfZero, IntermediaryCodeInstruction*),
    (CodeCall, Identifier, ArgumentList*)
);

typedef struct IntermediaryCode {
  IntermediaryCodeInstruction instruction;
  struct IntermediaryCode* next;
} IntermediaryCode;

IntermediaryCode* make_intermediary_code(Program program);
void print_intermediary_code(FILE* out, IntermediaryCode* code);

#endif