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

SemanticErrorList* concat_errors(SemanticErrorList* a, SemanticErrorList* b) {
  if (a == NULL) {
    return b;
  }

  if (b == NULL) {
    return a;
  }

  a->next = concat_errors(a->next, b);
  return a;
}

datatype(DeclarationSearchResult, (DeclarationNotFound), (DeclarationFound, Declaration, Type, Identifier));
datatype(ImplementationSearchResult, (ImplementationNotFound), (ImplementationFound, Implementation));

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

ImplementationSearchResult find_implementation(Identifier target, ImplementationList* implementations) {
  if (implementations == NULL) {
    return ImplementationNotFound();
  }

  if (strcmp(implementations->implementation.name, target) == 0) {
    return ImplementationFound(implementations->implementation);
  }

  return find_implementation(target, implementations->next);
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

SemanticErrorList* verify_double_implementations(ImplementationList* implementations) {
  char error_message[999];
  SemanticErrorList* errors = NULL;

  while (implementations != NULL) {
    if (MATCHES(
            find_implementation(implementations->implementation.name, implementations->next), ImplementationFound
        )) {
      snprintf(
          error_message, sizeof(error_message), "identificador \"%s\" declarado mais de uma vez",
          implementations->implementation.name
      );
      errors = concat_errors(errors, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
    }

    implementations = implementations->next;
  }

  return errors;
}

// Types that no object can have, but expressions can yield
datatype(HigherOrderType, (IntegerHigher), (FloatHigher), (CharHigher), (BooleanHigher), (StringHigher));
datatype(ExpressionType, (InvalidType), (ValidType, HigherOrderType));
datatype(OptionExpressionType, (NoneExpressionType), (SomeExpressionType, ExpressionType));

int is_assignable_to(HigherOrderType storing, HigherOrderType stored) {
  return storing.tag == stored.tag || MATCHES(storing, IntegerHigher) && MATCHES(stored, CharHigher) ||
         MATCHES(storing, CharHigher) && MATCHES(stored, IntegerHigher);
}

const char* higher_to_string(HigherOrderType type) {
  match(type) {
    of(IntegerHigher) return "int";
    of(FloatHigher) return "float";
    of(CharHigher) return "char";
    of(BooleanHigher) return "bool";
    of(StringHigher) return "string";
  }

  return "UNKNOWN";
}

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
    if (MATCHES(left_type, CharHigher) && MATCHES(right_type, IntegerHigher) ||
        MATCHES(left_type, IntegerHigher) && MATCHES(right_type, CharHigher)) {
      return ValidType(IntegerHigher());
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
      //                                          vvvv Why clang-format is doing this is a mystery :O
      return get_binary_expression_type(*operator, ** left, **right, declarations);
    }
  }

  return InvalidType();
}

OptionExpressionType join_types(OptionExpressionType a, OptionExpressionType b) {
  match(a) {
    of(NoneExpressionType) return b;
    of(SomeExpressionType, unvalidated_type_a) {
      match(*unvalidated_type_a) {
        of(InvalidType) return a;
        of(ValidType, type_a) {
          match(b) {
            of(NoneExpressionType) return a;
            of(SomeExpressionType, unvalidated_type_b) {
              match(*unvalidated_type_b) {
                of(InvalidType) return b;
                of(ValidType, type_b) {
                  if (is_assignable_to(*type_a, *type_b)) {
                    return SomeExpressionType(ValidType(*type_a));
                  } else if (is_assignable_to(*type_b, *type_a)) {
                    return SomeExpressionType(ValidType(*type_b));
                  } else {
                    return SomeExpressionType(InvalidType());
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return SomeExpressionType(InvalidType());
}

SemanticErrorList* verify_expression(Expression expression, DeclarationList* declarations) {
  char error_message[999];
  SemanticErrorList* error = NULL;

  match(expression) {
    of(LiteralExpression, literal) { }
    of(IdentifierExpression, identifier) {
      DeclarationSearchResult searchResult = find_declaration(*identifier, declarations);
      match(searchResult) {
        of(DeclarationNotFound) {
          snprintf(error_message, sizeof(error_message), "identificador \"%s\" não encontrado", *identifier);
          error = concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
        }
        of(DeclarationFound, declaration) {
          match(*declaration) {
            of(VariableDeclaration) { }
            otherwise {
              snprintf(
                  error_message, sizeof(error_message), "identificador \"%s\" não-escalar aparece em expressão",
                  *identifier
              );
              error =
                  concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
            }
          }
        }
      }
    }
    of(ReadArrayExpression, identifier, index) {
      error = concat_errors(error, verify_expression(**index, declarations));

      ExpressionType index_type = get_expression_type(**index, declarations);
      match(index_type) {
        of(ValidType, higher) {
          if (!is_assignable_to(IntegerHigher(), *higher)) {
            snprintf(
                error_message, sizeof(error_message),
                "expressão do tipo %s usada para indexar vetor \"%s\", esperava int", higher_to_string(*higher),
                *identifier
            );
            error =
                concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
          }
        }
        otherwise { }
      }

      DeclarationSearchResult searchResult = find_declaration(*identifier, declarations);
      match(searchResult) {
        of(DeclarationNotFound) {
          snprintf(error_message, sizeof(error_message), "identificador \"%s\" não encontrado", *identifier);
          error = concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
        }
        of(DeclarationFound, declaration) {
          match(*declaration) {
            of(ArrayDeclaration) { }
            otherwise {
              snprintf(
                  error_message, sizeof(error_message), "identificador não-vetorial \"%s\" indexado como vetor",
                  *identifier
              );
              error =
                  concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
            }
          }
        }
      }
    }
    of(FunctionCallExpression, identifier, aarguments) {
      DeclarationSearchResult searchResult = find_declaration(*identifier, declarations);
      match(searchResult) {
        of(DeclarationNotFound) {
          snprintf(error_message, sizeof(error_message), "uso de função não-declarada \"%s\"", *identifier);
          error = concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
        }
        of(DeclarationFound, declaration) {
          match(*declaration) {
            of(FunctionDeclaration, type, _, pparameters) {
              ArgumentList* arguments = *aarguments;
              ParametersDeclaration* parameters = *pparameters;
              int neededArguments = 0, passedArguments = 0;

              while (arguments != NULL && parameters != NULL) {
                ExpressionType type = get_expression_type(arguments->argument, declarations);
                match(type) {
                  of(ValidType, higher) {
                    if (!is_assignable_to(type_to_higher(parameters->type), *higher)) {
                      snprintf(
                          error_message, sizeof(error_message),
                          "tipo %s passado para \"%s\" na chamada da função \"%s\", esperava %s",
                          higher_to_string(*higher), parameters->name, *identifier,
                          higher_to_string(type_to_higher(parameters->type))
                      );
                      error = concat_errors(
                          error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) })
                      );
                    }
                  }
                  otherwise { }
                }

                arguments = arguments->next;
                parameters = parameters->next;
                neededArguments++;
                passedArguments++;
              }

              while (arguments != NULL) {
                arguments = arguments->next;
                passedArguments++;
              }
              while (parameters != NULL) {
                parameters = parameters->next;
                neededArguments++;
              }

              if (neededArguments != passedArguments) {
                snprintf(
                    error_message, sizeof(error_message), "\"%s\" esperava %d argumentos, mas recebeu %d", *identifier,
                    neededArguments, passedArguments
                );
                error = concat_errors(
                    error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) })
                );
              }
            }
            otherwise {
              snprintf(
                  error_message, sizeof(error_message), "identificador não-chamável \"%s\" usado como função",
                  *identifier
              );
              error =
                  concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
            }
          }
        }
      }
    }
    of(InputExpression, type) { }
    of(BinaryExpression, operator, left, right) {
      error = concat_errors(error, verify_expression(**left, declarations));
      error = concat_errors(error, verify_expression(**right, declarations));

      ExpressionType left_type = get_expression_type(**left, declarations);
      ExpressionType right_type = get_expression_type(**right, declarations);

      match(left_type) {
        of(ValidType, left_higher) {
          match(right_type) {
            of(ValidType, right_higher) {
              if (!is_assignable_to(*left_higher, *right_higher)) {
                snprintf(
                    error_message, sizeof(error_message), "expressão binária com tipos incompatíveis: %s e %s",
                    higher_to_string(*left_higher), higher_to_string(*right_higher)
                );
                error = concat_errors(
                    error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) })
                );
              }
            }
            otherwise { }
          }
        }
        otherwise { }
      }
    }
  }

  return error;
}

SemanticErrorList* verify_statement(Statement statement, DeclarationList* declarations) {
  char error_message[999];
  SemanticErrorList* error = NULL;

  match(statement) {
    of(AssignmentStatement, identifier, value) {
      error = concat_errors(error, verify_expression(*value, declarations));
      DeclarationSearchResult search_result = find_declaration(*identifier, declarations);
      match(search_result) {
        of(DeclarationNotFound) {
          snprintf(error_message, sizeof(error_message), "identificador \"%s\" não encontrado", *identifier);
          error = concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
        }
        of(DeclarationFound, declaration) {
          match(*declaration) {
            of(VariableDeclaration, type) {
              HigherOrderType variable_type = type_to_higher(*type);
              ExpressionType value_maybe_type = get_expression_type(*value, declarations);
              match(value_maybe_type) {
                of(ValidType, value_type) {
                  if (!is_assignable_to(variable_type, *value_type)) {
                    snprintf(
                        error_message, sizeof(error_message),
                        "impossível atribuir valor do tipo %s à variável \"%s\" do tipo %s",
                        higher_to_string(*value_type), *identifier, higher_to_string(variable_type)
                    );
                    error = concat_errors(
                        error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) })
                    );
                    return error;
                  }
                }
                otherwise { }
              }
            }
            otherwise {
              snprintf(error_message, sizeof(error_message), "atribuição à símbolo não-variável \"%s\"", *identifier);
              error =
                  concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
            }
          }
        }
      }
    }
    of(ArrayAssignmentStatement, identifier, index, value) {
      error = concat_errors(error, verify_expression(*value, declarations));
      error = concat_errors(error, verify_expression(*index, declarations));

      ExpressionType index_maybe_type = get_expression_type(*index, declarations);
      match(index_maybe_type) {
        of(ValidType, higher) {
          if (!is_assignable_to(IntegerHigher(), *higher)) {
            snprintf(
                error_message, sizeof(error_message), "impossível usar tipo %s no acesso ao vetor \"%s\"",
                higher_to_string(*higher), *identifier
            );
            error =
                concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
          }
        }
        otherwise { }
      }

      DeclarationSearchResult search_result = find_declaration(*identifier, declarations);
      match(search_result) {
        of(DeclarationNotFound) {
          snprintf(error_message, sizeof(error_message), "identificador \"%s\" não encontrado", *identifier);
          error = concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
        }
        of(DeclarationFound, declaration) {
          match(*declaration) {
            of(ArrayDeclaration, type) {
              HigherOrderType variable_type = type_to_higher(*type);
              ExpressionType value_maybe_type = get_expression_type(*value, declarations);
              match(value_maybe_type) {
                of(ValidType, value_type) {
                  if (!is_assignable_to(variable_type, *value_type)) {
                    snprintf(
                        error_message, sizeof(error_message),
                        "impossível atribuir valor do tipo %s a índice da variável \"%s\" do tipo %s[]",
                        higher_to_string(*value_type), *identifier, higher_to_string(variable_type)
                    );
                    error = concat_errors(
                        error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) })
                    );
                    return error;
                  }
                }
                otherwise { }
              }
            }
            otherwise {
              snprintf(
                  error_message, sizeof(error_message), "atribuição indexada a valor não-vetorial \"%s\"", *identifier
              );
              error =
                  concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
            }
          }
        }
      }
    }
    of(PrintStatement, expr) error = concat_errors(error, verify_expression(*expr, declarations));
    of(ReturnStatement, expr) error = concat_errors(error, verify_expression(*expr, declarations));
    of(IfStatement, cond, true_branch) {
      error = verify_expression(*cond, declarations);

      ExpressionType cond_type = get_expression_type(*cond, declarations);
      match(cond_type) {
        of(ValidType, higher) {
          if (!MATCHES(*higher, BooleanHigher)) {
            snprintf(error_message, sizeof(error_message), "condição não booleana em bloco if");
            error =
                concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
          }
        }
        otherwise { }
      }

      error = concat_errors(error, verify_statement(**true_branch, declarations));
    }
    of(IfElseStatement, cond, true_branch, false_branch) {
      error = verify_expression(*cond, declarations);

      ExpressionType cond_type = get_expression_type(*cond, declarations);
      match(cond_type) {
        of(ValidType, higher) {
          if (!MATCHES(*higher, BooleanHigher)) {
            snprintf(error_message, sizeof(error_message), "condição não booleana em bloco if");
            error =
                concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
          }
        }
        otherwise { }
      }

      error = concat_errors(error, verify_statement(**true_branch, declarations));
      error = concat_errors(error, verify_statement(**false_branch, declarations));
    }
    of(WhileStatement, cond, body) {
      error = verify_expression(*cond, declarations);

      ExpressionType cond_type = get_expression_type(*cond, declarations);
      match(cond_type) {
        of(ValidType, higher) {
          if (!MATCHES(*higher, BooleanHigher)) {
            snprintf(error_message, sizeof(error_message), "condição não booleana em bloco while");
            error =
                concat_errors(error, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
          }
        }
        otherwise { }
      }

      error = concat_errors(error, verify_statement(**body, declarations));
    }
    of(BlockStatement, llist) {
      StatementList* list = *llist;
      while (list != NULL) {
        error = concat_errors(error, verify_statement(list->statement, declarations));
        list = list->next;
      }
    }
    of(EmptyStatement) { }
  }

  return error;
}

int does_statement_always_return(Statement statement, DeclarationList* declarations) {
  match(statement) {
    of(ReturnStatement) { return 1; }
    of(IfElseStatement, _, true_block, false_block) {
      return does_statement_always_return(**true_block, declarations) &&
             does_statement_always_return(**false_block, declarations);
    }
    of(BlockStatement, s) {
      StatementList* list = *s;
      // If at least 1 statement is garanteed to return, we're safe
      while (list != NULL) {
        if (does_statement_always_return(list->statement, declarations)) {
          return 1;
        }
        list = list->next;
      }

      return 0;
    }
    otherwise { return 0; }
  }

  return 0;
}

SemanticErrorList*
verify_implementation_all_branches_return(Implementation implementation, DeclarationList* declarations) {
  char error_message[999];
  SemanticErrorList* errors = NULL;

  if (!does_statement_always_return(implementation.body, declarations)) {
    snprintf(error_message, sizeof(error_message), "função \"%s\" contém ramos sem retorno", implementation.name);
    errors = concat_errors(errors, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
  }

  return errors;
}

SemanticErrorList* verify_statement_return_types(
    Statement statement, Identifier function_identifier, Type expected_return, DeclarationList* declarations
) {
  char error_message[999];
  SemanticErrorList* errors = NULL;

  match(statement) {
    of(ReturnStatement, expr) {
      ExpressionType expr_type = get_expression_type(*expr, declarations);
      match(expr_type) {
        of(ValidType, higher) {
          if (!is_assignable_to(type_to_higher(expected_return), *higher)) {
            snprintf(
                error_message, sizeof(error_message), "retorno do tipo %s é inválido para função \"%s\" do tipo %s",
                higher_to_string(*higher), function_identifier, higher_to_string(type_to_higher(expected_return))
            );
            errors =
                concat_errors(errors, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
          }
        }
        otherwise { }
      }
    }
    of(IfStatement, _, block) errors = concat_errors(
        errors, verify_statement_return_types(**block, function_identifier, expected_return, declarations)
    );
    of(IfElseStatement, _, true_block, false_block) {
      errors = concat_errors(
          errors, verify_statement_return_types(**true_block, function_identifier, expected_return, declarations)
      );
      errors = concat_errors(
          errors, verify_statement_return_types(**false_block, function_identifier, expected_return, declarations)
      );
    }
    of(WhileStatement, _, block) errors = concat_errors(
        errors, verify_statement_return_types(**block, function_identifier, expected_return, declarations)
    );
    of(BlockStatement, s) {
      StatementList* list = *s;
      while (list != NULL) {
        errors = concat_errors(
            errors, verify_statement_return_types(list->statement, function_identifier, expected_return, declarations)
        );
        list = list->next;
      }
    }
    otherwise { }
  }

  return errors;
}

DeclarationList* concat_params(ParametersDeclaration* params, DeclarationList* declarations) {
  DeclarationList* head = NULL;
  DeclarationList* tail = NULL;

  while (params != NULL) {
    // HACK: Put a non-type-matching initial value inside the declaration
    DeclarationList* new_node = make_declaration(VariableDeclaration(params->type, params->name, IntLiteral(0)));

    if (tail != NULL) {
      tail->next = new_node;
    }
    tail = new_node;

    if (head == NULL) {
      head = tail;
    }

    params = params->next;
  }

  return head != NULL ? head : declarations;
}

SemanticErrorList* verify_implementation(Implementation implementation, DeclarationList* declarations) {
  // TODO: Prepend declarations with function parameters?

  char error_message[999];
  SemanticErrorList* errors = NULL;

  DeclarationSearchResult declaration_result = find_declaration(implementation.name, declarations);
  match(declaration_result) {
    of(DeclarationFound, declaration) {
      match(*declaration) {
        of(FunctionDeclaration, function_type, _, params) {
          DeclarationList* context = concat_params(*params, declarations);

          errors = concat_errors(
              errors,
              verify_statement_return_types(implementation.body, implementation.name, *function_type, context)
          );
          errors = concat_errors(errors, verify_implementation_all_branches_return(implementation, context));
          errors = concat_errors(errors, verify_statement(implementation.body, context));
        }
        otherwise {
          snprintf(
              error_message, sizeof(error_message), "função \"%s\" implementada mas declarada com tipo não-função",
              implementation.name
          );
          errors =
              concat_errors(errors, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
        }
      }
      of(DeclarationNotFound) {
        snprintf(
            error_message, sizeof(error_message), "função \"%s\" implementada mas não declarada", implementation.name
        );
        errors = concat_errors(errors, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
      }
    }
  }

  return errors;
}

SemanticErrorList* verify_missing_implementation(DeclarationList* declarations, ImplementationList* implementations) {
  char error_message[999];
  SemanticErrorList* errors = NULL;

  while (declarations != NULL) {
    match(declarations->declaration) {
      of(FunctionDeclaration, _, identifier) {
        ImplementationSearchResult result = find_implementation(*identifier, implementations);
        match(result) {
          of(ImplementationNotFound) {
            snprintf(error_message, sizeof(error_message), "função \"%s\" declarada mas não implementada", *identifier);
            errors =
                concat_errors(errors, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
          }
          otherwise { }
        }
      }
      otherwise { }
    }

    declarations = declarations->next;
  }

  return errors;
}

SemanticErrorList* verify_program(Program program) {
  SemanticErrorList* errors = NULL;

  errors = concat_errors(errors, verify_missing_implementation(program.declarations, program.implementations));
  errors = concat_errors(errors, verify_double_implementations(program.implementations));
  errors = concat_errors(errors, verify_double_declarations(program.declarations));

  ImplementationList* implementations = program.implementations;
  while (implementations != NULL) {
    errors = concat_errors(errors, verify_implementation(implementations->implementation, program.declarations));

    implementations = implementations->next;
  }

  return errors;
}
