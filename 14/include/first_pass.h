#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "assembler.h"
#include <stdio.h>

typedef struct {
    unsigned int opcode : 4;
    unsigned int src_addressing : 4;
    unsigned int dst_addressing : 4;
    unsigned int are : 3;
} Instruction;

int first_pass(const char* filename, SymbolTable* symbol_table);
int get_instruction_length(const char* instruction);

#endif /* FIRST_PASS_H */