#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include "assembler.h"

int second_pass(const char* filename, SymbolTable* symbol_table, MemoryImage* memory_image);
void write_output_files(const char* filename, MemoryImage* memory_image, SymbolTable* symbol_table);

#endif // SECOND_PASS_H