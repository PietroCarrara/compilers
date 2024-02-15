#include "asm.h"

#include "intermediary-code.h"

#include <string.h>

#define space()      fprintf(out, " ")
#define tabs(n)      fprintf(out, "%*s", (n)*TAB_SIZE, "")
#define character(c) fprintf(out, "%c", c)
#define string(s)    fprintf(out, "%s", s)
#define integer(i)   fprintf(out, "%d", i)
#define floating(f)  fprintf(out, "%g", f);

static void print_literal(Literal literal, FILE* out) {
  match(literal) {
    of(IntLiteral, i) fprintf(out, "%d", *i);
    of(FloatLiteral, f) fprintf(out, "%g", *f);
    of(CharLiteral, c) fprintf(out, "'%c'", *c);
    of(StringLiteral, s) fprintf(out, "\"%s\"", *s);
  }
}

static void print_type(Type type, FILE* out) {
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

int count_lines(FILE* out) {
  int line_number = 0;

  rewind(out);

  char ch = fgetc(out);
  while (!feof(out)) {
    if (ch == '\n') {
      line_number++;
    }
    ch = fgetc(out);
  }

  return line_number;
}

// Optimization to remember what value was inside each register and only load if necessary
void write_mov(FILE* out, char* src, char* dst) {
  // goto end; // Uncomment to turn off this optimization

  static int last_instruction = -1;
  int current_instruction = count_lines(out);

  static char* last_src = NULL;
  static char* last_dst = NULL;

  // Assert we're the very next instruction
  if (current_instruction - 1 != last_instruction) {
    goto end;
  }

  // Assert there are available labels
  if (last_src == NULL || last_dst == NULL) {
    goto end;
  }

  // We're moving back and forth! No need to do it
  if (strcmp(last_src, dst) == 0 && strcmp(last_dst, src) == 0) {
    last_instruction = current_instruction;
    last_src = src;
    last_dst = dst;
    return;
  }

end:
  fprintf(out, "mov %s, %s\n", src, dst);
  last_instruction = current_instruction;
  last_src = src;
  last_dst = dst;
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
        write_mov(out, *storage, "%r10d");
        fprintf(out, "test %%r10d, %%r10d\n");
        fprintf(out, "je %s\n", *label);
      }
      of(ICCopy, dst, src) {
        write_mov(out, *src, "%r10d");
        write_mov(out, "%r10d", *dst);
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
        fprintf(out, "pushq %%rbp\n"); // Setup a stack frame
        fprintf(out, "callq %s\n", *name);
        fprintf(out, "mov %%eax, %s\n", *dst); // TODO: Is this enough? Maybe we need per-type return values?
        fprintf(out, "popq %%rbp\n");
      }
      of(ICInput, type, dst) {
        char* format = NULL;
        match(*type) {
          of(IntegerType) format = "percent_d";
          of(FloatType) format = "percent_f";
          of(CharType) format = "percent_c";
        }
        fprintf(out, "pushq %%rbp\n"); // Setup a stack frame
        fprintf(out, "leaq %s(%%rip), %%rdi\n", format);
        fprintf(out, "movq %s@GOTPCREL(%%rip), %%rsi\n", *dst);
        fprintf(out, "movb $0, %%al\n");
        fprintf(out, "callq __isoc99_scanf@PLT\n");
        fprintf(out, "popq %%rbp\n");
      }
      of(ICPrint, src) {
        fprintf(out, "pushq %%rbp\n"); // Setup a stack frame
        fprintf(out, "leaq percent_s(%%rip), %%rdi\n");
        fprintf(out, "leaq %s(%%rip), %%rsi\n", *src);
        fprintf(out, "movb $0, %%al\n");
        fprintf(out, "callq printf@PLT\n");
        fprintf(out, "popq %%rbp\n");
      }
      of(ICReturn, src) {
        fprintf(out, "mov %s, %%eax\n", *src);
        fprintf(out, "retq\n");
      }
      of(ICBinOp, operator, dst, left, right) {
        write_mov(out, *left, "%r10d");

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
          of(AndOperator) fprintf(out, "and %s, %%r10d\n", *right);
          of(OrOperator) fprintf(out, "or %s, %%r10d\n", *right);
          of(NotOperator) fprintf(out, "xor %s, %%r10d\n", *right);
          of(LessOrEqualOperator) {
            fprintf(out, "mov %s, %%r11d\n", *right);
            fprintf(out, "cmp %%r11d, %%r10d\n");
            fprintf(out, "mov $0, %%eax\n");
            fprintf(out, "setle %%al\n");
            fprintf(out, "mov %%eax, %%r10d\n");
          }
          of(GreaterOrEqualOperator) {
            fprintf(out, "mov %s, %%r11d\n", *right);
            fprintf(out, "cmp %%r11d, %%r10d\n");
            fprintf(out, "mov $0, %%eax\n");
            fprintf(out, "setge %%al\n");
            fprintf(out, "mov %%eax, %%r10d\n");
          };
          of(EqualsOperator) {
            fprintf(out, "mov %s, %%r11d\n", *right);
            fprintf(out, "cmp %%r11d, %%r10d\n");
            fprintf(out, "mov $0, %%eax\n");
            fprintf(out, "sete %%al\n");
            fprintf(out, "mov %%eax, %%r10d\n");
          }
          of(DiffersOperator) {
            fprintf(out, "mov %s, %%r11d\n", *right);
            fprintf(out, "cmp %%r11d, %%r10d\n");
            fprintf(out, "mov $0, %%eax\n");
            fprintf(out, "setne %%al\n");
            fprintf(out, "mov %%eax, %%r10d\n");
          };
        }
        write_mov(out, "%r10d", *dst);
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
      otherwise { }
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
  string("percent_s: .asciz \"%s\"\n");
  string("percent_d: .asciz \"%d\"\n");
  string("percent_f: .asciz \"%f\"\n");
  string("percent_c: .asciz \"%c\"\n");
  string("\n");

  string(".text\n");
  write_intermediary_code(ic, out);
  string("\n");

  string(".section \".note.GNU-stack\",\"\",@progbits\n");
}