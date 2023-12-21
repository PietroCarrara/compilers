#include "semantic-check.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SemanticErrorList* make_semantic_error_list(SemanticError error) {
  SemanticErrorList* list = malloc(sizeof(SemanticErrorList));

  list->error = error;
  list->next = NULL;

  return list;
}

datatype(DeclarationSearchResult, (DeclarationNotFound), (DeclarationFound, Declaration, Type, Identifier));

DeclarationSearchResult find_declaration(Identifier target, DeclarationList* declarations) {
  if (declarations == NULL) {
    return DeclarationNotFound();
  }

  Identifier identifier;
  Type type;
  match(declarations->declaration) {
    of(VariableDeclaration, t, i) {
      identifier = *i;
      type = *t;
    }
    of(FunctionDeclaration, t, i) {
      identifier = *i;
      type = *t;
    }
    of(ArrayDeclaration, t, i) {
      identifier = *i;
      type = *t;
    }
  }

  if (strcmp(identifier, target) == 0) {
    return DeclarationFound(declarations->declaration, type, identifier);
  }

  return find_declaration(target, declarations->next);
}

SemanticErrorList* verify_double_declarations(DeclarationList* declarations) {
  if (declarations == NULL) {
    return NULL;
  }

  Declaration declaration = declarations->declaration;

  Identifier identifier;
  match(declaration) {
    of(VariableDeclaration, _, i) identifier = *i;
    of(FunctionDeclaration, _, i) identifier = *i;
    of(ArrayDeclaration, _, i) identifier = *i;
  }

  SemanticErrorList* error = NULL;
  SemanticErrorList* next_errors = verify_double_declarations(declarations->next);

  if (MATCHES(find_declaration(identifier, declarations->next), DeclarationFound)) {
    char error_message[999];
    snprintf(error_message, sizeof(error_message), "identificador \"%s\" declarado mais de uma vez", identifier);

    error = make_semantic_error_list((SemanticError) { .message = strdup(error_message) });
    error->next = next_errors;
  }

  return error != NULL ? error : next_errors;
}

// Types that no object can have, but expressions can yield
datatype(HigherOrderType, (IntegerHigher), (FloatHigher), (CharHigher), (BooleanHigher), (StringHigher));
datatype(ExpressionType, (InvalidType), (ValidType, HigherOrderType));

HigherOrderType type_to_higher(Type type) {
  match(type) {
    of(IntegerType) return IntegerHigher();
    of(FloatType) return FloatHigher();
    of(CharType) return CharHigher();
  }

  // Should never happen
  return IntegerHigher();
}

ExpressionType get_expression_type(Expression expression, DeclarationList* declarations);

ExpressionType
get_binary_expression_type(BinaryOperator operator, Expression left, Expression right, DeclarationList* declarations) {
  ExpressionType left_result = get_expression_type(left, declarations);
  ExpressionType right_result = get_expression_type(right, declarations);

  HigherOrderType left_type, right_type;
  match(left_result) {
    of(ValidType, type) left_type = *type;
    otherwise return InvalidType();
  }
  match(right_result) {
    of(ValidType, type) right_type = *type;
    otherwise return InvalidType();
  }

  int is_arithmetic = MATCHES(operator, SumOperator) || MATCHES(operator, SubtractionOperator) ||
                      MATCHES(operator, MultiplicationOperator) || MATCHES(operator, DivisionOperator);
  int is_comparison = MATCHES(operator, LessThanOperator) || MATCHES(operator, GreaterThanOperator) ||
                      MATCHES(operator, GreaterOrEqualOperator) || MATCHES(operator, EqualsOperator) ||
                      MATCHES(operator, DiffersOperator);
  int is_logic = MATCHES(operator, AndOperator) || MATCHES(operator, OrOperator) || MATCHES(operator, NotOperator) ||
                 MATCHES(operator, LessOrEqualOperator);

  if (is_arithmetic) {
    if (MATCHES(left_type, CharHigher) && MATCHES(right_type, CharHigher)) {
      return ValidType(CharHigher());
    }
    if (MATCHES(left_type, IntegerHigher) && MATCHES(right_type, IntegerHigher)) {
      return ValidType(IntegerHigher());
    }
    if (MATCHES(left_type, FloatHigher) && MATCHES(right_type, FloatHigher)) {
      return ValidType(FloatHigher());
    }
    return InvalidType();
  }

  if (is_comparison) {
    if (left_type.tag == right_type.tag) {
      return ValidType(BooleanHigher());
    }
    return InvalidType();
  }

  return InvalidType();
}

ExpressionType get_expression_type(Expression expression, DeclarationList* declarations) {
  match(expression) {
    of(LiteralExpression, literal) {
      match(*literal) {
        of(IntLiteral) return ValidType(IntegerHigher());
        of(FloatLiteral) return ValidType(FloatHigher());
        of(CharLiteral) return ValidType(CharHigher());
        of(StringLiteral) return ValidType(StringHigher());
      }
    }
    of(IdentifierExpression, identifier) {
      DeclarationSearchResult searchResult = find_declaration(*identifier, declarations);
      match(searchResult) {
        of(DeclarationNotFound) return InvalidType();
        of(DeclarationFound, declaration) {
          match(*declaration) {
            of(VariableDeclaration, type) return ValidType(type_to_higher(*type));
            otherwise return InvalidType();
          }
        }
      }
    }
    of(ReadArrayExpression, identifier) {
      DeclarationSearchResult searchResult = find_declaration(*identifier, declarations);
      match(searchResult) {
        of(DeclarationNotFound) return InvalidType();
        of(DeclarationFound, declaration) {
          match(*declaration) {
            of(ArrayDeclaration, type) return ValidType(type_to_higher(*type));
            otherwise return InvalidType();
          }
        }
      }
    }
    of(FunctionCallExpression, identifier) {
      DeclarationSearchResult searchResult = find_declaration(*identifier, declarations);
      match(searchResult) {
        of(DeclarationNotFound) return InvalidType();
        of(DeclarationFound, declaration) {
          match(*declaration) {
            of(FunctionDeclaration, type) return ValidType(type_to_higher(*type));
            otherwise return InvalidType();
          }
        }
      }
    }
    of(InputExpression, type) return ValidType(type_to_higher(*type));
    of(BinaryExpression, operator, left, right) {
      //                                            vvv Why clang-format is doing this is a mystery :O
      return get_binary_expression_type(*operator, ** left, **right, declarations);
    }
  }

  return InvalidType();
}

SemanticErrorList* verify_statement(Statement statement) { return NULL; }

SemanticErrorList* verify_program(Program program) { return verify_double_declarations(program.declarations); }