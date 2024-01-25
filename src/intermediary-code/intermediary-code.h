#ifndef INTERMEDIARY_CODE_H
#define INTERMEDIARY_CODE_H

#include "syntax-tree.h"

#include <datatype99.h>
#include <stdio.h>

typedef char* Label;
typedef char* Storage;

datatype(
    IC, (ICNoop), (ICVariable, Storage, Literal), (ICArrayVariable, Storage, ArrayInitialization*), (ICJump, Label),
    (ICJumpIfFalse, Storage, Label), (ICCopy, Storage, Storage), (ICCopyAt, Storage, Storage, Storage),
    (ICCopyFrom, Storage, Storage, Storage), (ICCall, Identifier, Storage), (ICInput, Type, Storage),
    (ICBinOp, BinaryOperator, Storage, Storage, Storage), (ICPrint, Storage), (ICReturn, Storage),
    // TODO: Do I really need these ones?
    (ICFunctionBegin, Identifier), (ICFunctionEnd)
);

typedef struct IntermediaryCode {
  struct IntermediaryCode* next;
  Label label;
  IC instruction;
} IntermediaryCode;

IntermediaryCode* intemediary_code_from_program(Program);
void print_intermediary_code(IntermediaryCode*);

#endif