#include <string.h>
#include <stdio.h>
#include "symbol_table.h"

void initSymbolTable(SymbolTable *table) {
    table->count = 0;
}

int addSymbol(SymbolTable *table, const char *label, int address) {
    if (table->count >= MAX_SYMBOLS) {
        printf("Error: Symbol table overflow. Cannot add more symbols.\n");
        return -1;
    }
    if (symbolExists(table, label)) {
        printf("Error: Duplicate symbol '%s'.\n", label);
        return -1;
    }
    strncpy(table->symbols[table->count].label, label, LABEL_LENGTH - 1);
    table->symbols[table->count].label[LABEL_LENGTH - 1] = '\0';
    table->symbols[table->count].address = address;
    table->count++;
    return 0;
}

int getSymbolAddress(const SymbolTable *table, const char *label) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].label, label) == 0) {
            return table->symbols[i].address;
        }
    }
    printf("Error: Symbol '%s' not found.\n", label);
    return -1; // Return an invalid address
}

int symbolExists(const SymbolTable *table, const char *label) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].label, label) == 0) {
            return 1;
        }
    }
    return 0;
}