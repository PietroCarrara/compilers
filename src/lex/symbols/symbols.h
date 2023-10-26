#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <datatype99.h>

datatype(
  Symbol, (IntLiteral, int), (FloatLiteral, float), (CharLiteral, char), (StringLiteral, char*),
  (IdentifierLiteral, char*)
);

typedef struct {
  char* name;
  Symbol symbol;
} SymbolData;

datatype(SymbolSearchResult, (SymbolNotFound), (SymbolFound, SymbolData));

// Registers a symbol
void register_symbol(SymbolData data);
// Finds a registered symbol according to its name
SymbolSearchResult find_symbol(char* name);
// Applies a callback to each registered symbol
void foreach_symbol(void (*callback)(SymbolData*));

#endif