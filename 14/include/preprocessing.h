#ifndef PREPROCESSING_H
#define PREPROCESSING_H

// Struct for a macro
typedef struct {
    char* name;
    char** lines;
    int line_count;
} Macro;

// Struct for a macro table (list of macros)
typedef struct {
    Macro* items;
    int count;
    int capacity;
} MacroTable;

// Function prototypes
MacroTable create_macro_table();
void add_macro(MacroTable* table, const char* name, char** lines, int line_count);
Macro* find_macro(MacroTable* table, const char* name);
void free_macro_table(MacroTable* table);
int preprocess_file(const char* filename);

#endif // PREPROCESSING_H