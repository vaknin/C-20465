/***************************************************************
 * File: preprocessing.h
 * Description: Defines structures and functions for macro preprocessing.
 ***************************************************************/

#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <stdio.h>
#include "helper_functions.h"

/* Struct for a macro */
typedef struct {
    char* name;
    char** lines;
    int line_count;
} Macro;

/* Struct for a macro table (list of macros) */
typedef struct {
    Macro* items;
    int count;
    int capacity;
} MacroTable;

/* create_macro_table: Initializes and returns an empty macro table. */
MacroTable create_macro_table(void);

/* add_macro: Adds a new macro to the macro table. */
void add_macro(MacroTable* table, const char* name, char** lines, int line_count);

/* find_macro: Searches for a macro by name in the macro table. */
Macro* find_macro(MacroTable* table, const char* name);

/* free_macro_table: Frees all memory associated with the macro table. */
void free_macro_table(MacroTable* table);

/* trim_end: Removes trailing whitespace from a string. */
char* trim_end(char* str);

/* tokenize_line: Splits a line into tokens. */
char** tokenize_line(const char* line, int* token_count);

/* trim_start: Removes leading whitespace from a string. */
char* trim_start(char* str);

/* preprocess_file: Performs macro preprocessing on the input file. */
int preprocess_file(const char* filename);

#endif /* PREPROCESSING_H */