#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_SYMBOLS 100
#define LABEL_LENGTH 31

// Structure to represent a symbol
typedef struct {
    char label[LABEL_LENGTH]; // Label name (max 30 characters + null terminator)
    int address;              // Corresponding memory address
} Symbol;

// Structure for the symbol table
typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    int count;
} SymbolTable;

// Function declarations
void initSymbolTable(SymbolTable *table);
int addSymbol(SymbolTable *table, const char *label, int address);
int getSymbolAddress(const SymbolTable *table, const char *label);
int symbolExists(const SymbolTable *table, const char *label);

#endif // SYMBOL_TABLE_H