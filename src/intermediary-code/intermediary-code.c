#include "intermediary-code.h"

#include "syntax-tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Label* make_label(const char* label) {
  Label* l = malloc(sizeof(Label));
  *l = strdup(label);
  return l;
}

Label* next_label() {
  static int label_count = 0;

  char label[999];
  sprintf(label, "label_%d", label_count);
  label_count++;

  return make_label(label);
}

IntermediaryCode* make_code(IntermediaryCodeInstruction instruction) {
  IntermediaryCode* code = malloc(sizeof(IntermediaryCode));
  code->instruction = instruction;
  code->label = NULL;
  code->next = NULL;
  return code;
}

IntermediaryCode* append_code(IntermediaryCode* code, IntermediaryCode* appending) {
  if (code == NULL) {
    return appending;
  }

  if (code->next == NULL) {
    code->next = appending;
    return code;
  }

  append_code(code->next, appending);
  return code;
}

IntermediaryCode* make_expression_body(Expression expr) { return make_code(CodeNop()); }

IntermediaryCode* make_implementation_body(Statement body, Label *after_label, int* used_after_label) {
  *used_after_label = 0;

  match(body) {
    of(AssignmentStatement, id, expr) {
      IntermediaryCode* expr_body = make_expression_body(*expr);
      return append_code(expr_body, make_code(CodeStore(*id)));
    }
    of(ArrayAssignmentStatement, id, index, value) {
      IntermediaryCode* value_body = make_expression_body(*value);
      IntermediaryCode* index_body = make_expression_body(*index);
      return append_code(append_code(value_body, index_body), make_code(CodeStoreArray(*id)));
    }
    of(PrintStatement, expr) {
      IntermediaryCode* expr_body = make_expression_body(*expr);
      return append_code(expr_body, make_code(CodePrint()));
    }
    of(ReturnStatement, expr) {
      IntermediaryCode* expr_body = make_expression_body(*expr);
      return append_code(expr_body, make_code(CodeReturnValue()));
    }
    of(IfStatement, expr, true_branch) {
      *used_after_label = 1;
      IntermediaryCode* expr_body = make_expression_body(*expr);
      return append_code(make_code(CodeJumpIfFalse(after_label)), expr_body);
    }
    of(IfElseStatement, expr, true_branch, false_branch) {
      *used_after_label = 1;
      IntermediaryCode* expr_body = make_expression_body(*expr);

      return append_code(make_code(CodeJumpIfFalse(after_label)), expr_body);
    }
    of(WhileStatement) { }
    of(BlockStatement, statement_list) {
      IntermediaryCode* body = NULL;
      StatementList* list = *statement_list;

      // HACK: I don't like this way to generate future labels for instructions that need them (like forward jumps)...
      Label* previous_label = NULL;
      Label* current_label = NULL;
      int previous_used = 0;
      int current_used = 0;

      while (list != NULL) {
        previous_label = current_label;
        current_label = list->next != NULL ? next_label() : after_label;
        previous_used = current_used;

        IntermediaryCode* instr = make_implementation_body(list->statement, current_label, &current_used);
        if (previous_used) {
          instr->label = previous_label;
        }

        body = append_code(body, instr);

        list = list->next;
      }
      if (body != NULL) {
        return body;
      }
    }
    of(EmptyStatement) { }
  }

  // Never supposed to happen
  return make_code(CodeNop());
}

IntermediaryCode* make_implementations(ImplementationList* implementations) {
  IntermediaryCode* result = NULL;
  while (implementations != NULL) {
    // Build function instructions, and append a return after each function
    int used;
    Label* label = next_label();
    IntermediaryCode* body_code = make_implementation_body(implementations->implementation.body, label, &used);
    IntermediaryCode* final_return = make_code(CodeReturn());
    if (used) {
      final_return->label = label;
    }
    body_code = append_code(body_code, final_return);

    if (body_code->label == NULL) {
      body_code->label = make_label(implementations->implementation.name);
      result = append_code(result, body_code);
    } else {
      IntermediaryCode* wrapper = make_code(CodeNop());
      wrapper->label = make_label(implementations->implementation.name);
      wrapper = append_code(wrapper, body_code);
      result = append_code(result, wrapper);
    }

    implementations = implementations->next;
  }

  return result;
}

IntermediaryCode* make_declarations(DeclarationList* declarations) {
  IntermediaryCode* result = NULL;
  while (declarations != NULL) {
    IntermediaryCode* code = make_code(CodeDeclaration(declarations->declaration));
    result = append_code(result, code);

    declarations = declarations->next;
  }

  return result;
}

IntermediaryCode* make_intermediary_code(Program program) {
  IntermediaryCode* result = NULL;
  result = append_code(result, make_declarations(program.declarations));
  result = append_code(result, make_implementations(program.implementations));
  return result;
}
