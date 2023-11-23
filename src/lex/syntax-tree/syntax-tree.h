#ifndef SYNTAX_TREE_H 
#define SYNTAX_TREE_H 

#include <datatype99.h>

typedef char* Identifier;

datatype(
    Type,
    (IntegerType),
    (FloatType),
    (CharType)
);

typedef struct ParametersDeclaration {
    Type type;
    Identifier name;
    struct ParametersDeclaration* next;
} ParametersDeclaration;

datatype(
    Expression,
    (SumExpression, Expression*, Expression*)
);

datatype(
    Statement,
    (VariableDeclaration, Type, Identifier, Expression), // type, name, initial value
    (FunctionDefinition, Type, Identifier, ParametersDeclaration*, Statement*) // type, name, parameters, function body
);

typedef struct StatementList {
    Statement statement;
    struct StatementList* next;
} StatementList;


#endif