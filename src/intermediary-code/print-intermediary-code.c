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
      string(": const ");
      print_literal(out, *literal);
    }
    of(ArrayDeclaration) { }
    of(FunctionDeclaration) { }
  }
}

void print_intermediary_code(FILE* out, IntermediaryCode* code) {
  while (code != NULL) {
    match(code->instruction) { of(CodeDeclaration, declaration) print_declaration_code(out, *declaration); }

    code = code->next;
  }
}