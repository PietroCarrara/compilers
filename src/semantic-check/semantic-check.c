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
      //                                            vvv Why clang-format is doing this is a mystery :O
      return get_binary_expression_type(*operator, ** left, **right, declarations);
    }
  }

  return InvalidType();
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

SemanticErrorList* verify_implementation(Implementation implementation, DeclarationList* declarations) {
  char error_message[999];
  SemanticErrorList* errors = NULL;
  errors = concat_errors(errors, verify_statement(implementation.body, declarations));

  DeclarationSearchResult declaration_result = find_declaration(implementation.name, declarations);
  match(declaration_result) {
    of(DeclarationFound, declaration) {
      // TODO: Check return typing and match agains function type
    }
    of(DeclarationNotFound) {
      snprintf(
          error_message, sizeof(error_message), "função \"%s\" implementada mas não declarada", implementation.name
      );
      errors = concat_errors(errors, make_semantic_error_list((SemanticError) { .message = strdup(error_message) }));
    }
  }

  return errors;
}

SemanticErrorList* verify_program(Program program) {
  SemanticErrorList* errors = NULL;

  // TODO: Check that every declared function is implemented
  errors = concat_errors(errors, verify_double_declarations(program.declarations));

  ImplementationList* implementations = program.implementations;
  while (implementations != NULL) {
    errors = concat_errors(errors, verify_implementation(implementations->implementation, program.declarations));

    implementations = implementations->next;
  }

  return errors;
}