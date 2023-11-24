#include "format.h"

#define space() fprintf(out, " ")
#define character(c)  fprintf(out, "%c", c)
#define string(s)  fprintf(out, "%s", s)
#define integer(i) fprintf(out, "%d", i)

void print_literal(FILE* out, Literal literal) {
  match (literal) {
    of(IntLiteral, i) fprintf(out, "%d", *i);
    of(FloatLiteral, f) fprintf(out, "%g", *f);
    of(CharLiteral, c) fprintf(out, "'%c'", *c);
    of(StringLiteral, s) fprintf(out, "\"%s\"", *s); // TODO: Fix string extra quotes
  }
}

void print_identifier(FILE* out, Identifier identifier) {
  string(identifier);
}

void print_type(FILE* out, Type type) {
  match(type) {
    of(IntegerType) string("int");
    of(FloatType) string("float");
    of(CharType) string("char");
  }
}

void print_parameters_declaration(FILE* out, ParametersDeclaration* params) {
  if (params == NULL) {
    return;
  }

  print_type(out, params->type);
  space();
  print_identifier(out, params->name);
  if (params->next != NULL) {
    string(", ");
    print_parameters_declaration(out, params->next);
  }
}

void print_array_initialization(FILE* out, ArrayInitialization* values) {
  if (values == NULL) {
    return;
  }

  print_literal(out, values->value);
  if (values->next != NULL) {
    space();
    print_array_initialization(out, values->next);
  }
}

void print_variable_declaration(FILE* out, Type type, Identifier name, Literal value) {
  print_type(out, type);
  space();
  print_identifier(out, name);
  string(" = ");
  print_literal(out, value);
  string(";\n");
}

void print_function_declaration(FILE* out, Type type, Identifier name, ParametersDeclaration* params) {
  print_type(out, type);
  space();
  print_identifier(out, name);

  character('(');
  print_parameters_declaration(out, params);
  character(')');

  string(";\n");
}

void print_array_declaration(FILE* out, Type type, Identifier name, int size, ArrayInitialization* values) {
  print_type(out, type);
  space();
  print_identifier(out, name);
  fprintf(out, "[%d]", size);
  if (values != NULL) {
    string(" = ");
    print_array_initialization(out, values);
  }
  string(";\n");
}

void print_declarations(FILE* out, DeclarationList* declarations) {
  if (declarations == NULL) {
    return;
  }

  match(declarations->declaration) {
    of(VariableDeclaration, type, identifier, literal) print_variable_declaration(out, *type, *identifier, *literal);
    of(FunctionDeclaration, type, identifier, parameters) print_function_declaration(out, *type, *identifier, *parameters);
    of(ArrayDeclaration, type, identifier, size, values) print_array_declaration(out, *type, *identifier, *size, *values);
  }

  print_declarations(out, declarations->next);
}

void print_program(FILE* out, Program program) {
  print_declarations(out, program.declarations);
}