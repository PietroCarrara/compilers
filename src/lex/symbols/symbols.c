#include "symbols.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct self {
  struct self* next;
  SymbolData data;
} SymbolDataListNode;

static SymbolDataListNode* hash_table = NULL;

void register_symbol(SymbolData data) {
  SymbolDataListNode** current = &hash_table;

  // Find the first empty node of the list
  while (*current != NULL) {
    current = &(*current)->next;
  }

  // Append to list end
  SymbolDataListNode* node = malloc(sizeof(SymbolDataListNode));
  node->data = data;
  node->next = NULL;
  *current = node;
}

SymbolSearchResult find_symbol(char* name) {
  SymbolDataListNode** current = &hash_table;

  while (*current != NULL) {
    SymbolData current_node = (*current)->data;

    if (strcmp(name, (current_node.name)) == 0) {
      return SymbolFound(current_node);
    }

    current = &(*current)->next;
  }

  return SymbolNotFound();
}

void foreach_symbol(void (*callback)(SymbolData*)) {
  SymbolDataListNode** current = &hash_table;

  while (*current != NULL) {
    SymbolData* current_node = &(*current)->data;
    callback(current_node);
    current = &(*current)->next;
  }
}
