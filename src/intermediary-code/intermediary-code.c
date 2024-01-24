#include "intermediary-code.h"

#include "syntax-tree.h"

#include <stdlib.h>
#include <string.h>

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

IntermediaryCode* make_intermediary_code_expression(Expression expr, Storage* result) {
  match(expr) {
    of(LiteralExpression, literal) {
      char buffer[256];

      // HACK: Name the storage for literals the same as their formatted value
      match(*literal) {
        of(IntLiteral, i) snprintf(buffer, sizeof(buffer), "%d", *i);
        of(FloatLiteral, f) snprintf(buffer, sizeof(buffer), "%g", *f);
        of(CharLiteral, c) snprintf(buffer, sizeof(buffer), "'%c'", *c);
        of(StringLiteral, s) snprintf(buffer, sizeof(buffer), "\"%s\"", *s);
      };
      *result = strdup(buffer);
      return make_ic(ICNoop()); // TODO: This won't be a noop in the future
    }

    otherwise {
      *result = next_storage();
      return make_ic(ICNoop()); // TODO: Implement
    }
  }
}

IntermediaryCode* make_intermediary_code(const StatementList* current) {
  if (current == NULL) {
    return make_ic(ICNoop());
  }

  match(current->statement) {
    of(AssignmentStatement, identifier, expr) {
      Storage expr_result;
      IntermediaryCode* expression = make_intermediary_code_expression(*expr, &expr_result);
      IntermediaryCode* rest = make_intermediary_code(current->next);

      IntermediaryCode* result = NULL;
      result = concat_ic(result, expression);
      result = concat_ic(result, make_ic(ICCopy(*identifier, expr_result)));
      result = concat_ic(result, rest);
      return result;
    }
    of(ArrayAssignmentStatement, identifier, index_expr, expr) {
      Storage expr_result;
      IntermediaryCode* expression = make_intermediary_code_expression(*expr, &expr_result);
      Storage index_expr_result;
      IntermediaryCode* index_expression = make_intermediary_code_expression(*index_expr, &index_expr_result);

      IntermediaryCode* rest = make_intermediary_code(current->next);

      IntermediaryCode* result = NULL;
      result = concat_ic(result, expression);
      result = concat_ic(result, index_expression);
      result = concat_ic(result, make_ic(ICCopyAt(*identifier, index_expr_result, expr_result)));
      result = concat_ic(result, rest);
      return result;
    }
    of(PrintStatement) {
      // TODO: Implement
      return make_intermediary_code(current->next);
    }
    of(ReturnStatement) {
      // TODO: Implement
      return make_intermediary_code(current->next);
    }
    of(IfStatement, cond, true_statement) {
      Storage condition_result;
      IntermediaryCode* condition = make_intermediary_code_expression(*cond, &condition_result);
      IntermediaryCode* true_branch = make_intermediary_code(from_statement(**true_statement));
      IntermediaryCode* rest = with_label(make_intermediary_code(current->next), next_label());

      IntermediaryCode* result = NULL;
      result = concat_ic(result, condition);
      result = concat_ic(result, make_ic(ICJumpIfFalse(condition_result, rest->label)));
      result = concat_ic(result, true_branch);
      result = concat_ic(result, rest);
      return result;
    }
    of(IfElseStatement, cond, true_statement, false_statement) {
      Storage condition_result;
      IntermediaryCode* condition = make_intermediary_code_expression(*cond, &condition_result);
      IntermediaryCode* true_branch = make_intermediary_code(from_statement(**true_statement));
      IntermediaryCode* false_branch =
          with_label(make_intermediary_code(from_statement(**true_statement)), next_label());
      IntermediaryCode* rest = with_label(make_intermediary_code(current->next), next_label());

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
          with_label(make_intermediary_code_expression(*cond, &condition_result), next_label());
      IntermediaryCode* loop_body = make_intermediary_code(from_statement(**body));
      IntermediaryCode* rest = with_label(make_intermediary_code(current->next), next_label());

      IntermediaryCode* result = NULL;
      result = concat_ic(result, condition);
      result = concat_ic(result, make_ic(ICJumpIfFalse(condition_result, rest->label)));
      result = concat_ic(result, loop_body);
      result = concat_ic(result, make_ic(ICJump(condition->label)));
      result = concat_ic(result, rest);
      return result;
    }
    of(BlockStatement, list) { return concat_ic(make_intermediary_code(*list), make_intermediary_code(current->next)); }
    of(EmptyStatement) { return make_intermediary_code(current->next); }
  }

  // Should never happen
  return NULL;
}

IntermediaryCode* intemediary_code_from_program(Program program) {
  IntermediaryCode* result = NULL;

  ImplementationList* implementations = program.implementations;
  while (implementations != NULL) {
    result = concat_ic(result, make_intermediary_code(from_statement(implementations->implementation.body)));
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
      of(ICVariable) { }
      of(ICArrayVariable) { }
      of(ICJump, label) { printf("JUMP(goto = %s)\n", *label); }
      of(ICJumpIfFalse, storage, label) { printf("JUMP_IF_FALSE(read = %s, goto = %s)\n", *storage, *label); }
      of(ICCopy, dst, src) { printf("COPY(destination = %s, source = %s)\n", *dst, *src); }
      of(ICCopyAt, dst, idx, src) {
        printf("COPY_TO_ARRAY(destination = %s, index = %s, source = %s)\n", *dst, *idx, *src);
      }
    }

    code = code->next;
  }
}