#include "asm.h"

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
        string("_");
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
    }

    declarations = declarations->next;
  }
}

void write_asm(Program program, FILE* out) {
  string(".global main\n");
  string("\n");

  write_declarations(program.declarations, out);
  string("\n");

  string(".text\n");
  // TODO: ASM functions here
  string("\n");

  string(".section \".note.GNU - stack \",\"\",@progbits\n");
}