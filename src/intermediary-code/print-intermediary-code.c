#include "intermediary-code.h"

#define space()      fprintf(out, " ")
#define tabs(n)      fprintf(out, "%*s", (n)*TAB_SIZE, "")
#define character(c) fprintf(out, "%c", c)
#define string(s)    fprintf(out, "%s", s)
#define integer(i)   fprintf(out, "%d", i)

void print_literal(FILE* out, Literal literal) {
  match(literal) {
    of(IntLiteral, i) fprintf(out, "%d", *i);
    of(FloatLiteral, f) fprintf(out, "%g", *f);
    of(CharLiteral, c) fprintf(out, "'%c'", *c);
    of(StringLiteral, s) fprintf(out, "\"%s\"", *s);
  }
}

void print_identifier(FILE* out, Identifier identifier) { string(identifier); }

void print_declaration_code(FILE* out, Declaration declaration) {
  match(declaration) {
    of(VariableDeclaration, _, identifier, literal) {
      print_identifier(out, *identifier);
      string(": value ");
      print_literal(out, *literal);
      string("\n");
    }
    of(ArrayDeclaration, _, identifier, size, initial_values) {
      print_identifier(out, *identifier);
      string(":\n");
      if (size == 0) {
        string("value 0\n");
      } else {
        int i = 0;
        ArrayInitialization* values = *initial_values;
        for (int i = 0; i < *size; i++) {
          if (values != NULL) {
            string("value ");
            print_literal(out, values->value);
            string("\n");
            values = values->next;
          } else {
            string("value 0\n");
          }
        }
      }
    }
    of(FunctionDeclaration) { } // Do nothing
  }
}

void print_intermediary_code(FILE* out, IntermediaryCode* code) {
  while (code != NULL) {
    if (code->label != NULL) {
      string(*(code->label));
      string(":\n");
    }

    match(code->instruction) {
      of(CodeDeclaration, declaration) print_declaration_code(out, *declaration);
      of(CodeStore, identifier) {
        string("store ");
        print_identifier(out, *identifier);
        string("\n");
      }
      of(CodeStoreArray, identifier) {
        string("store_at_position ");
        print_identifier(out, *identifier);
        string("\n");
      }
      of(CodeReturnValue) { string("return_value\n"); }
      of(CodePrint) { string("print\n"); }
      of(CodeReturn) { string("return\n"); }
      of(CodeJumpIfFalse, label) { string("jump_if_false "); string(**label); string("\n"); }
    }

    code = code->next;
  }
}