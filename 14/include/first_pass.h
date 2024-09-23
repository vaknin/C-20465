#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "assembler.h"
#include "helper_functions.h"
#include <stdio.h>

int first_pass(const char* filename, SymbolTable* symbol_table);
int get_instruction_length(const char* instruction);

#endif /* FIRST_PASS_H */