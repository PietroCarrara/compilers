#include "intermediary-code.h"

#include "semantic-check.h"
#include "syntax-tree.h"

#include <stdlib.h>
#include <string.h>

StringDeclarationList* string_constants = NULL;

Label next_label() {
  static int current_label = 0;

  char buffer[256];
  snprintf(buffer, sizeof(buffer), "label_%d", current_label);
  current_label++;

  return strdup(buffer);
}

Storage next_storage() {
  static int current_storage = 0;

  char buffer[256];
  snprintf(buffer, sizeof(buffer), "storage_%d", current_storage);
  current_storage++;

  return strdup(buffer);
}

IntermediaryCode* make_ic(IC instruction) {
  IntermediaryCode* ic = malloc(sizeof(IntermediaryCode));
  ic->next = NULL;
  ic->instruction = instruction;
  ic->label = NULL;
  return ic;
}

IntermediaryCode* concat_ic(IntermediaryCode* first, IntermediaryCode* second) {
  if (first == NULL) {
    return second;
  }
  if (second == NULL) {
    return first;
  }

  if (first->next == NULL) {
    first->next = second;
    return first;
  }

  first->next = concat_ic(first->next, second);
  return first;
}

IntermediaryCode* with_label(IntermediaryCode* ic, Label label) {
  if (ic->label != NULL) {
    IntermediaryCode* noop = make_ic(ICNoop());
    noop->label = label;
    return concat_ic(noop, ic);
  }

  ic->label = label;
  return ic;
}

StatementList* from_statement(Statement statement) {
  match(statement) {
    of(BlockStatement, list) { return *list; }
    otherwise {
      StatementList* list = malloc(sizeof(StatementList));
      list->next = NULL;
      list->statement = statement;
      return list;
    }
  }

  // Not supposed to happen
  return NULL;
}

IntermediaryCode* make_intermediary_code_expression(Expression expr, Storage* result, DeclarationList* declarations) {
  // HACK: These two cases name their storage as custom values
  if (!MATCHES(expr, LiteralExpression) && !MATCHES(expr, IdentifierExpression)) {
    *result = next_storage();
  }

  match(expr) {
    of(LiteralExpression, literal) {
      char buffer[256];

      // HACK: Name the storage for literals the same as their formatted value
      match(*literal) {
        of(IntLiteral, i) snprintf(buffer, sizeof(buffer), "$%d", *i);
        of(FloatLiteral, f) snprintf(buffer, sizeof(buffer), "$%g", *f);
        of(CharLiteral, c) snprintf(buffer, sizeof(buffer), "$'%c'", *c);
        of(StringLiteral, s) {
          Storage storage = next_storage();
          snprintf(buffer, sizeof(buffer), "%s", storage);

          // HACK: Register string constant on global list
          StringDeclarationList declaration = { .identifier = storage, .value = *s, .next = NULL };
          StringDeclarationList* tail = string_constants;
          while (string_constants != NULL && tail->next != NULL) {
            tail = tail->next;
          }
          if (string_constants == NULL) {
            string_constants = malloc(sizeof(StringDeclarationList));
            *string_constants = declaration;
          } else {
            tail->next = malloc(sizeof(StringDeclarationList));
            *(tail->next) = declaration;
          }
        }
      }
      *result = strdup(buffer);
      return make_ic(ICNoop()); // TODO: This won't be a noop in the future
    }
    of(IdentifierExpression, identifier) {
      *result = strdup(*identifier); // HACK: Name the storage for identifier the same as their name
      return make_ic(ICNoop());      // TODO: This won't be a noop in the future
    }
    of(ReadArrayExpression, identifier, index_expression) {
      Storage index_result = NULL;
      IntermediaryCode* index_code = make_intermediary_code_expression(**index_expression, &index_result, declarations);
      IntermediaryCode* read_code = make_ic(ICCopyFrom(*result, *identifier, index_result));

      return concat_ic(index_code, read_code);
    }
    of(FunctionCallExpression, function_identifier, arguments) {
      DeclarationSearchResult search_function = find_declaration(*function_identifier, declarations);
      match(search_function) {
        of(DeclarationFound, declaration) {
          match(*declaration) {
            of(FunctionDeclaration, _, _, params) {
              IntermediaryCode* call_result = NULL;

              ArgumentList* arguments_list = *arguments;
              ParametersDeclaration* parameters_list = *params;
              while (arguments_list != NULL && parameters_list != NULL) {
                Storage arg_result = NULL;

                call_result = concat_ic(
                    call_result, make_intermediary_code_expression(arguments_list->argument, &arg_result, declarations)
                );
                call_result = concat_ic(call_result, make_ic(ICCopy(parameters_list->name, arg_result)));

                arguments_list = arguments_list->next;
                parameters_list = parameters_list->next;
              }
              return concat_ic(call_result, make_ic(ICCall(*function_identifier, *result)));
            }
          }
        }
      }
    }
    of(InputExpression, type) { return make_ic(ICInput(*type, *result)); }
    of(BinaryExpression, operator, left, right) {
      IntermediaryCode* binop_result = NULL;
      Storage left_result = NULL;
      Storage right_result = NULL;

      binop_result = concat_ic(binop_result, make_intermediary_code_expression(**left, &left_result, declarations));
      binop_result = concat_ic(binop_result, make_intermediary_code_expression(**right, &right_result, declarations));
      binop_result = concat_ic(binop_result, make_ic(ICBinOp(*operator, * result, left_result, right_result)));

      return binop_result;
    }
  }
}

IntermediaryCode* make_intermediary_code(const StatementList* current, DeclarationList* declarations) {
  if (current == NULL) {
    return make_ic(ICNoop());
  }

  match(current->statement) {
    of(AssignmentStatement, identifier, expr) {
      Storage expr_result;
      IntermediaryCode* expression = make_intermediary_code_expression(*expr, &expr_result, declarations);
      IntermediaryCode* rest = make_intermediary_code(current->next, declarations);

      IntermediaryCode* result = NULL;
      result = concat_ic(result, expression);
      result = concat_ic(result, make_ic(ICCopy(*identifier, expr_result)));
      result = concat_ic(result, rest);
      return result;
    }
    of(ArrayAssignmentStatement, identifier, index_expr, expr) {
      Storage expr_result;
      IntermediaryCode* expression = make_intermediary_code_expression(*expr, &expr_result, declarations);
      Storage index_expr_result;
      IntermediaryCode* index_expression =
          make_intermediary_code_expression(*index_expr, &index_expr_result, declarations);

      IntermediaryCode* rest = make_intermediary_code(current->next, declarations);

      IntermediaryCode* result = NULL;
      result = concat_ic(result, expression);
      result = concat_ic(result, index_expression);
      result = concat_ic(result, make_ic(ICCopyAt(*identifier, index_expr_result, expr_result)));
      result = concat_ic(result, rest);
      return result;
    }
    of(PrintStatement, expr) {
      Storage expr_result;
      IntermediaryCode* expression = make_intermediary_code_expression(*expr, &expr_result, declarations);
      IntermediaryCode* rest = make_intermediary_code(current->next, declarations);

      IntermediaryCode* result = NULL;
      result = concat_ic(result, expression);
      result = concat_ic(result, make_ic(ICPrint(expr_result)));
      result = concat_ic(result, rest);
      return result;
    }
    of(ReturnStatement, expr) {
      Storage expr_result;
      IntermediaryCode* expression = make_intermediary_code_expression(*expr, &expr_result, declarations);
      IntermediaryCode* rest = make_intermediary_code(current->next, declarations);

      IntermediaryCode* result = NULL;
      result = concat_ic(result, expression);
      result = concat_ic(result, make_ic(ICReturn(expr_result)));
      result = concat_ic(result, rest);
      return result;
    }
    of(IfStatement, cond, true_statement) {
      Storage condition_result;
      IntermediaryCode* condition = make_intermediary_code_expression(*cond, &condition_result, declarations);
      IntermediaryCode* true_branch = make_intermediary_code(from_statement(**true_statement), declarations);
      IntermediaryCode* rest = with_label(make_intermediary_code(current->next, declarations), next_label());

      IntermediaryCode* result = NULL;
      result = concat_ic(result, condition);
      result = concat_ic(result, make_ic(ICJumpIfFalse(condition_result, rest->label)));
      result = concat_ic(result, true_branch);
      result = concat_ic(result, rest);
      return result;
    }
    of(IfElseStatement, cond, true_statement, false_statement) {
      Storage condition_result;
      IntermediaryCode* condition = make_intermediary_code_expression(*cond, &condition_result, declarations);
      IntermediaryCode* true_branch = make_intermediary_code(from_statement(**true_statement), declarations);
      IntermediaryCode* false_branch =
          with_label(make_intermediary_code(from_statement(**false_statement), declarations), next_label());
      IntermediaryCode* rest = with_label(make_intermediary_code(current->next, declarations), next_label());

      IntermediaryCode* result = NULL;
      result = concat_ic(result, condition);
      result = concat_ic(result, make_ic(ICJumpIfFalse(condition_result, false_branch->label)));
      result = concat_ic(result, true_branch);
      result = concat_ic(result, make_ic(ICJump(rest->label)));
      result = concat_ic(result, false_branch);
      result = concat_ic(result, rest);
      return result;
    }
    of(WhileStatement, cond, body) {
      Storage condition_result;
      IntermediaryCode* condition =
          with_label(make_intermediary_code_expression(*cond, &condition_result, declarations), next_label());
      IntermediaryCode* loop_body = make_intermediary_code(from_statement(**body), declarations);
      IntermediaryCode* rest = with_label(make_intermediary_code(current->next, declarations), next_label());

      IntermediaryCode* result = NULL;
      result = concat_ic(result, condition);
      result = concat_ic(result, make_ic(ICJumpIfFalse(condition_result, rest->label)));
      result = concat_ic(result, loop_body);
      result = concat_ic(result, make_ic(ICJump(condition->label)));
      result = concat_ic(result, rest);
      return result;
    }
    of(BlockStatement, list) {
      return concat_ic(
          make_intermediary_code(*list, declarations), make_intermediary_code(current->next, declarations)
      );
    }
    of(EmptyStatement) { return make_intermediary_code(current->next, declarations); }
  }

  // Should never happen
  return NULL;
}

IntermediaryCode* intemediary_code_from_program(Program program) {
  IntermediaryCode* result = NULL;

  ImplementationList* implementations = program.implementations;
  while (implementations != NULL) {
    result = concat_ic(result, make_ic(ICFunctionBegin(implementations->implementation.name)));
    result = concat_ic(
        result, make_intermediary_code(from_statement(implementations->implementation.body), program.declarations)
    );
    result = concat_ic(result, make_ic(ICFunctionEnd()));
    implementations = implementations->next;
  }

  return result;
}

void print_intermediary_code(IntermediaryCode* code) {
  while (code != NULL) {
    if (code->label != NULL) {
      printf("LABEL(name = %s)\n", code->label);
    }

    match(code->instruction) {
      of(ICNoop) { }
      of(ICFunctionBegin, name) printf("FUNCTION_BEGIN(name = %s)\n", *name);
      of(ICFunctionEnd) printf("FUNCTION_END()\n");
      of(ICJump, label) { printf("JUMP(goto = %s)\n", *label); }
      of(ICJumpIfFalse, storage, label) { printf("JUMP_IF_FALSE(read = %s, goto = %s)\n", *storage, *label); }
      of(ICCopy, dst, src) { printf("COPY(destination = %s, source = %s)\n", *dst, *src); }
      of(ICCopyAt, dst, idx, src) { printf("COPY_TO_ARRAY(destination = %s[%s], source = %s)\n", *dst, *idx, *src); }
      of(ICCopyFrom, dst, src, idx) {
        printf("COPY_FROM_ARRAY(destination = %s, source = %s[%s])\n", *dst, *src, *idx);
      }
      of(ICCall, name, dst) printf("CALL(identifier = %s, destination = %s)\n", *name, *dst);
      of(ICInput, type, dst) {
        printf("INPUT(type = ");
        match(*type) {
          of(IntegerType) printf("INT");
          of(FloatType) printf("FLOAT");
          of(CharType) printf("CHAR");
        }
        printf(", destination = %s)\n", *dst);
      }
      of(ICPrint, src) printf("PRINT(src = %s)\n", *src);
      of(ICReturn, src) printf("RETURN(src = %s)\n", *src);
      of(ICBinOp, operator, dst, left, right) {
        match(*operator) {
          of(SumOperator) printf("SUM");
          of(SubtractionOperator) printf("SUB");
          of(MultiplicationOperator) printf("MUL");
          of(DivisionOperator) printf("DIV");
          of(LessThanOperator) printf("LT");
          of(GreaterThanOperator) printf("GT");
          of(AndOperator) printf("AND");
          of(OrOperator) printf("OR");
          of(NotOperator) printf("NOT");
          of(LessOrEqualOperator) printf("LE");
          of(GreaterOrEqualOperator) printf("GE");
          of(EqualsOperator) printf("EQUALS");
          of(DiffersOperator) printf("DIFFERS");
        }
        printf("(destination = %s, operand_left = %s, operand_right = %s)\n", *dst, *left, *right);
      }
    }

    code = code->next;
  }
}