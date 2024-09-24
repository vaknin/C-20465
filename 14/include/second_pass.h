/***************************************************************
 * File: second_pass.h
 * Description: Declares functions for the second pass of the assembler.
 ***************************************************************/

#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include "assembler.h"
#include "helper_functions.h"

/* second_pass: Performs the second pass of the assembly process. */
int second_pass(const char* filename, SymbolTable* symbol_table, MemoryImage* memory_image);

/* write_output_files: Generates output files based on the assembled code. */
void write_output_files(const char* filename, MemoryImage* memory_image, SymbolTable* symbol_table);

#endif /* SECOND_PASS_H */