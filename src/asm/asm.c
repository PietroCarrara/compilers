#include "asm.h"

#include "intermediary-code.h"

#define space()      fprintf(out, " ")
#define tabs(n)      fprintf(out, "%*s", (n)*TAB_SIZE, "")
#define character(c) fprintf(out, "%c", c)
#define string(s)    fprintf(out, "%s", s)
#define integer(i)   fprintf(out, "%d", i)
#define floating(f)  fprintf(out, "%g", f);

void print_literal(Literal literal, FILE* out) {
  match(literal) {
    of(IntLiteral, i) fprintf(out, "%d", *i);
    of(FloatLiteral, f) fprintf(out, "%g", *f);
    of(CharLiteral, c) fprintf(out, "'%c'", *c);
    of(StringLiteral, s) fprintf(out, "\"%s\"", *s);
  }
}

void print_type(Type type, FILE* out) {
  match(type) {
    of(IntegerType) string(".int");
    of(FloatType) string(".float");
    of(CharType) string(".int");
  }
}

void write_declarations(DeclarationList* declarations, FILE* out) {
  while (declarations != NULL) {
    match(declarations->declaration) {
      of(VariableDeclaration, type, identifier, value) {
        string(*identifier);
        string(": ");
        print_type(*type, out);
        space();
        print_literal(*value, out);
        string("\n");
      }
      of(ArrayDeclaration, type, identifier, size, initialization) {
        string("_");
        string(*identifier);
        string(": ");

        int i = 0;
        ArrayInitialization* list = *initialization;
        while (i < *size || list != NULL) {
          print_type(*type, out);
          space();

          if (list != NULL) {
            print_literal(list->value, out);
            list = list->next;
          } else {
            integer(0);
          }
          i++;
          string("\n");
        }
      }
      of(FunctionDeclaration, _, _, parameters) {
        ParametersDeclaration* paramsList = *parameters;
        while (paramsList != NULL) {
          string(paramsList->name);
          string(": ");
          print_type(paramsList->type, out);
          string(" 0\n");

          paramsList = paramsList->next;
        }
      }
    }

    declarations = declarations->next;
  }
}

// HACK: Huuuge hack to declare string literals
extern StringDeclarationList* string_constants;

void write_string_literals(FILE* out) {
  StringDeclarationList* list = string_constants;
  while (list != NULL) {
    fprintf(out, "%s: .asciz \"%s\"\n", list->identifier, list->value);

    list = list->next;
  }
}

void write_intermediary_code(IntermediaryCode* code, FILE* out) {
  while (code != NULL) {
    if (code->label != NULL) {
      string(code->label);
      string(": ");
    }

    match(code->instruction) {
      of(ICNoop) { }
      of(ICFunctionBegin, name) {
        string(*name);
        string(": ");
      }
      of(ICFunctionEnd) string("retq # Function end\n\n");
      of(ICJump, label) fprintf(out, "jmp %s\n", *label);
      of(ICJumpIfFalse, storage, label) {
        fprintf(out, "mov %s, %%r10d\n", *storage);
        fprintf(out, "test %%r10d, %%r10d\n");
        fprintf(out, "je %s\n", *label);
      }
      of(ICCopy, dst, src) {
        fprintf(out, "mov %s, %%r10d\n", *src);
        fprintf(out, "mov %%r10d, %s\n", *dst);
      }
      of(ICCopyAt, dst, idx, src) {
        fprintf(out, "mov $%s, %%r10d\n", *dst);
        fprintf(out, "mov %s, %%r11d\n", *idx);
        fprintf(out, "mov %s, %%r11d(%%r10d)\n", *src);
      }
      of(ICCopyFrom, dst, src, idx) {
        fprintf(out, "mov $%s, %%r10d\n", *src);
        fprintf(out, "mov %s, %%r11d\n", *idx);
        fprintf(out, "mov %%r11d(%%r10d)\n, %s", *dst);
      }
      of(ICCall, name, dst) {
        fprintf(out, "callq %s\n", *name);
        fprintf(out, "mov %%eax, %s\n", *dst); // TODO: Is this enough? Maybe we need per-type return values?
      }
      of(ICInput, type, dst) {
        // TODO
        printf("INPUT(type = ");
        match(*type) {
          of(IntegerType) printf("INT");
          of(FloatType) printf("FLOAT");
          of(CharType) printf("CHAR");
        }
        printf(", destination = %s)\n", *dst);
      }
      of(ICPrint, src) {
        fprintf(out, "leaq percent_s(%%rip), %%rdi\n");
        fprintf(out, "leaq %s(%%rip), %%rsi\n", *src);
        fprintf(out, "movb $0, %%al\n");
        fprintf(out, "callq printf@PLT\n");
      }
      of(ICReturn, src) {
        fprintf(out, "mov %s, %%eax\n", *src);
        fprintf(out, "retq\n");
      }
      of(ICBinOp, operator, dst, left, right) {
        fprintf(out, "mov %s, %%r10d\n", *left);

        match(*operator) {
          of(SumOperator) fprintf(out, "add %s, %%r10d\n", *right);
          of(SubtractionOperator) fprintf(out, "sub %s, %%r10d\n", *right);
          of(MultiplicationOperator) fprintf(out, "imul %s, %%r10d\n", *right);
          of(DivisionOperator) {
            // FIXME: This is still leaving a leftover mov %r10d before it
            fprintf(out, "mov %s, %%eax\n", *left);
            fprintf(out, "cltd\n");
            fprintf(out, "mov %s, %%r10d\n", *right);
            fprintf(out, "idiv %%r10d\n");
            fprintf(out, "mov %%eax, %%r10d\n");
          }
          of(LessThanOperator) {
            fprintf(out, "mov %s, %%r11d\n", *right);
            fprintf(out, "cmp %%r11d, %%r10d\n");
            fprintf(out, "mov $0, %%eax\n");
            fprintf(out, "setb %%al\n");
            fprintf(out, "mov %%eax, %%r10d\n");
          }
          of(GreaterThanOperator) {
            fprintf(out, "mov %s, %%r11d\n", *right);
            fprintf(out, "cmp %%r11d, %%r10d\n");
            fprintf(out, "mov $0, %%eax\n");
            fprintf(out, "seta %%al\n");
            fprintf(out, "mov %%eax, %%r10d\n");
          }
          of(AndOperator) printf("AND");
          of(OrOperator) printf("OR");
          of(NotOperator) printf("NOT");
          of(LessOrEqualOperator) printf("LE");
          of(GreaterOrEqualOperator) printf("GE");
          of(EqualsOperator) {
            fprintf(out, "mov %s, %%r11d\n", *right);
            fprintf(out, "cmp %%r11d, %%r10d\n");
            fprintf(out, "mov $0, %%eax\n");
            fprintf(out, "sete %%al\n");
            fprintf(out, "mov %%eax, %%r10d\n");
          }
          of(DiffersOperator) printf("DIFFERS");
        }
        fprintf(out, "mov %%r10d, %s\n", *dst);
      }
    }

    code = code->next;
  }
}

void write_storage(IntermediaryCode* code, FILE* out) {
  while (code != NULL) {
    match(code->instruction) {
      of(ICCall, name, dst) fprintf(out, "%s: .int 0\n", *dst);
      of(ICInput, type, dst) fprintf(out, "%s: .int 0\n", *dst);
      of(ICBinOp, operator, dst, left, right) fprintf(out, "%s: .int 0\n", *dst);
    }

    code = code->next;
  }
}

void write_asm(Program program, FILE* out) {
  IntermediaryCode* ic = intemediary_code_from_program(program);

  string(".global main\n");
  string("\n");

  string(".data\n");
  write_declarations(program.declarations, out);
  string("\n");
  write_string_literals(out);
  string("\n");
  write_storage(ic, out);
  string("\n");
  string("percent_s: .asciz \"%s\"");
  string("\n");

  string(".text\n");
  write_intermediary_code(ic, out);
  string("\n");

  string(".section \".note.GNU-stack\",\"\",@progbits\n");
}