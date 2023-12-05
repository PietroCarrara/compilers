#include "format.h"

#define TAB_SIZE 2

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

void print_type(FILE* out, Type type) {
  match(type) {
    of(IntegerType) string("int");
    of(FloatType) string("float");
    of(CharType) string("char");
  }
}

void print_operator(FILE* out, BinaryOperator operator) {
  match (operator) {
    of (SumOperator) string("+");
    of (SubtractionOperator) string("-");
    of (MultiplicationOperator) string("*");
    of (DivisionOperator) string("/");
    of (LessThanOperator) string("<");
    of (GreaterThanOperator) string(">");
    of (AndOperator) string("&");
    of (OrOperator) string("|");
    of (NotOperator) string("~");
    of (LessOrEqualOperator) string("<=");
    of (GreaterOrEqualOperator) string(">=");
    of (EqualsOperator) string("==");
    of (DiffersOperator) string("!=");
  }
}

void print_expression(FILE* out, Expression expression);

// TODO: Tears come out of my eyes everytime I look at this horrible code
void print_operation_with_precedence(FILE* out, BinaryOperator operator, Expression left, Expression right) {
  int is_high_precedence = MATCHES(operator, MultiplicationOperator) || MATCHES(operator, DivisionOperator);

  char* left_prefix = "";
  char* left_postfix = "";
  if (is_high_precedence) {
    match (left) {
      of(BinaryExpression, left_operator) {
        if (MATCHES(*left_operator, SumOperator) || MATCHES(*left_operator, SubtractionOperator)) {
          left_prefix = "(";
          left_postfix = ")";
        }
      }
      otherwise {}
    }
  }
  string(left_prefix);
  print_expression(out, left);
  string(left_postfix);

  space();
  print_operator(out, operator);
  space();

  char* right_prefix = "";
  char* right_postfix = "";
  if (is_high_precedence) {
    match (right) {
      of(BinaryExpression, right_operator) {
        if (MATCHES(*right_operator, SumOperator) || MATCHES(*right_operator, SubtractionOperator)) {
          right_prefix = "(";
          right_postfix = ")";
        }
      }
      otherwise {}
    }
  }
  string(right_prefix);
  print_expression(out, right);
  string(right_postfix);
}

void print_argument_list(FILE* out, ArgumentList* arguments) {
  if (arguments == NULL) {
    return;
  }

  print_expression(out, arguments->argument);
  if (arguments->next != NULL) {
    string(", ");
    print_argument_list(out, arguments->next);
  }
}

void print_expression(FILE* out, Expression expression) {
  match(expression) {
    of(LiteralExpression, lit) print_literal(out, *lit);
    of(IdentifierExpression, name) print_identifier(out, *name);
    of(ReadArrayExpression, name, index) {
      print_identifier(out, *name);
      character('[');
      print_expression(out, **index);
      character(']');
    }
    of(FunctionCallExpression, name, argument_list) {
      print_identifier(out, *name);
      character('(');
      print_argument_list(out, *argument_list);
      character(')');
    }
    of(InputExpression, type) {
      string("input(");
      print_type(out, *type);
      character(')');
    }
    of(BinaryExpression, operator, left, right) print_operation_with_precedence(out, *operator, **left, **right);
  }
}

void print_statement(FILE* out, Statement statement, int level);

void print_block_statement(FILE* out, StatementList* body, int level) {
  if (body == NULL) {
    string("{ }");
    return;
  }

  string("{\n");
  while (body != NULL) {
    tabs(level + 1);
    print_statement(out, body->statement, level + 1);
    character('\n');
    body = body->next;
  }
  tabs(level);
  string("}");
}

void print_statement(FILE* out, Statement statement, int level) {
  match(statement) {
    of(AssignmentStatement, name, expr) {
      print_identifier(out, *name);
      string(" = ");
      print_expression(out, *expr);
      character(';');
    }
    of(ArrayAssignmentStatement, name, index, value) {
      print_identifier(out, *name);
      character('[');
      print_expression(out, *index);
      character(']');
      string(" = ");
      print_expression(out, *value);
      character(';');
    }
    of(PrintStatement, expr) {
      string("print ");
      print_expression(out, *expr);
      string(";");
    }
    of(ReturnStatement, expr) {
      string("return ");
      print_expression(out, *expr);
      character(';');
    }
    of(IfStatement, condition, body) {
      string("if (");
      print_expression(out, *condition);
      string(") ");
      print_statement(out, **body, level);
    }
    of(IfElseStatement, condition, true_body, false_body) {
      string("if (");
      print_expression(out, *condition);
      string(") ");
      print_statement(out, **true_body, level);
      string(" else ");
      print_statement(out, **false_body, level);
    }
    of(WhileStatement, condition, body) {
      string("while (");
      print_expression(out, *condition);
      string(") ");
      print_statement(out, **body, level);
    }
    of(BlockStatement, body) print_block_statement(out, *body, level);
    of(EmptyStatement) character(';');
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
    space();
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
    of(FunctionDeclaration, type, identifier, parameters)
        print_function_declaration(out, *type, *identifier, *parameters);
    of(ArrayDeclaration, type, identifier, size, values)
        print_array_declaration(out, *type, *identifier, *size, *values);
  }

  print_declarations(out, declarations->next);
}

void print_implementations(FILE* out, ImplementationList* implementations) {
  if (implementations == NULL) {
    return;
  }

  string("code ");
  print_identifier(out, implementations->implementation.name);
  space();
  print_statement(out, implementations->implementation.body, 0);
}

void print_program(FILE* out, Program program) {
  print_declarations(out, program.declarations);
  print_implementations(out, program.implementations);
}