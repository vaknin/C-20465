/***************************************************************
 * File: first_pass.h
 * Description: Header file for the first pass of the assembler.
 ***************************************************************/

#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "assembler.h"
#include "helper_functions.h"
#include <stdio.h>

/* first_pass: Performs the first pass of the assembler on the given file. */
int first_pass(const char* filename, SymbolTable* symbol_table);

/* get_instruction_length: Calculates the length of an instruction in words. */
int get_instruction_length(const char* instruction);

#endif /* FIRST_PASS_H */