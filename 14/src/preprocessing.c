/***************************************************************
 * File: preprocessing.c
 * Description: Implements macro preprocessing functionality.
 ***************************************************************/

#define _POSIX_C_SOURCE 200809L
#include "preprocessing.h"
#include "helper_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* create_macro_table: Initializes and returns an empty macro table. */
MacroTable create_macro_table(void)
{
    MacroTable table;
    table.items = NULL;
    table.count = 0;
    table.capacity = 0;
    return table;
}

/* add_macro: Adds a new macro to the macro table. */
void add_macro(MacroTable* table, const char* name, char** lines, int line_count)
{
    int i;
    Macro* macro;

    /* Expand the table if necessary */
    if (table->count == table->capacity) {
        table->capacity = table->capacity ? table->capacity * 2 : 1;
        table->items = (Macro*)realloc(table->items, table->capacity * sizeof(Macro));
    }
    
    macro = &table->items[table->count++];
    macro->name = strdup(name);
    macro->lines = (char**)malloc(line_count * sizeof(char*));
    macro->line_count = line_count;
    
    /* Copy each line of the macro */
    for (i = 0; i < line_count; i++) {
        macro->lines[i] = strdup(lines[i]);
    }
}

/* find_macro: Searches for a macro by name in the macro table. */
Macro* find_macro(MacroTable* table, const char* name)
{
    int i;
    for (i = 0; i < table->count; i++) {
        if (strcmp(table->items[i].name, name) == 0) {
            return &table->items[i];
        }
    }
    return NULL; /* Macro not found */
}

/* free_macro_table: Frees all memory associated with the macro table. */
void free_macro_table(MacroTable* table)
{
    int i, j;
    for (i = 0; i < table->count; i++) {
        free(table->items[i].name);
        for (j = 0; j < table->items[i].line_count; j++) {
            free(table->items[i].lines[j]);
        }
        free(table->items[i].lines);
    }
    free(table->items);
    table->items = NULL;
    table->count = 0;
    table->capacity = 0;
}

/* trim_end: Removes trailing whitespace from a string. */
char* trim_end(char* str)
{
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

/* tokenize_line: Splits a line into tokens. */
char** tokenize_line(const char* line, int* token_count)
{
    char** tokens = NULL;
    int capacity = 0;
    const char* start = line;
    const char* end;

    *token_count = 0;

    while (*start) {
        /* Skip leading whitespace */
        while (isspace((unsigned char)*start)) start++;
        if (*start == '\0') break;

        /* Find end of token */
        end = start;
        while (*end && !isspace((unsigned char)*end)) end++;

        /* Expand tokens array if necessary */
        if (*token_count == capacity) {
            capacity = capacity ? capacity * 2 : 1;
            tokens = (char**)realloc(tokens, capacity * sizeof(char*));
        }
        
        /* Add new token */
        tokens[(*token_count)++] = (char*)strndup(start, end - start);
        start = end;
    }

    return tokens;
}

/* trim_start: Removes leading whitespace from a string. */
char* trim_start(char* str)
{
    while (isspace((unsigned char)*str)) str++;
    return str;
}

/* preprocess_file: Performs macro preprocessing on the input file. */
int preprocess_file(const char* filename)
{
    char *input_filename, *output_filename, *base_filename;
    const char* extension;
    FILE *input, *output;
    MacroTable macro_table;
    char *line = NULL, *stripped_line, *trimmed, *macro_name = NULL, **macro_lines = NULL, *base_indent;
    size_t len = 0;
    int read;
    int macro_line_count = 0, macro_lines_capacity = 0;
    int in_macro = 0;
    int line_number = 0;
    int token_count, i, j, expanded;
    char** tokens;
    Macro* macro;

    /* Determine input and output filenames */
    extension = strrchr(filename, '.');
    
    if (extension && strcmp(extension, ".as") == 0) {
        input_filename = strdup(filename);
        base_filename = (char*)strndup(filename, extension - filename);
    } else {
        input_filename = (char*)malloc(strlen(filename) + 4);
        sprintf(input_filename, "%s.as", filename);
        base_filename = strdup(filename);
    }
    
    output_filename = (char*)malloc(strlen(base_filename) + 4);
    sprintf(output_filename, "%s.am", base_filename);

    /* Open input and output files */
    input = fopen(input_filename, "r");
    output = fopen(output_filename, "w");
    if (!input || !output) {
        report_error(0, "Error opening input or output file");
        free(input_filename);
        free(output_filename);
        free(base_filename);
        return 1;
    }

    macro_table = create_macro_table();

    /* Process input file line by line */
    while ((read = getline(&line, &len, input)) != -1) {
        line_number++;
        stripped_line = strip_comments(line);
        if (stripped_line[0] == '\0') {
            fprintf(output, "\n");  /* Preserve empty lines */
            continue;
        }

        trimmed = trim_end(strdup(stripped_line));
        tokens = tokenize_line(trimmed, &token_count);

        if (token_count > 0) {
            if (strcmp(tokens[0], "macr") == 0) {
                /* Start of macro definition */
                if (token_count != 2) {
                    report_error(line_number, "Invalid macro definition");
                } else if (in_macro) {
                    report_error(line_number, "Nested macro definitions are not allowed");
                } else {
                    free(macro_name);
                    macro_name = strdup(tokens[1]);
                    in_macro = 1;
                    macro_line_count = 0;
                }
            } else if (strcmp(tokens[0], "endmacr") == 0) {
                /* End of macro definition */
                if (!in_macro) {
                    report_error(line_number, "endmacr without corresponding macr");
                } else {
                    add_macro(&macro_table, macro_name, macro_lines, macro_line_count);
                    in_macro = 0;
                    for (i = 0; i < macro_line_count; i++) {
                        free(macro_lines[i]);
                    }
                    free(macro_lines);
                    macro_lines = NULL;
                    macro_line_count = macro_lines_capacity = 0;
                }
            } else if (in_macro) {
                /* Inside macro definition */
                if (macro_line_count == macro_lines_capacity) {
                    macro_lines_capacity = macro_lines_capacity ? macro_lines_capacity * 2 : 1;
                    macro_lines = (char**)realloc(macro_lines, macro_lines_capacity * sizeof(char*));
                }
                macro_lines[macro_line_count++] = strdup(trim_start(trimmed));
            } else {
                /* Normal line or macro expansion */
                expanded = 0;
                base_indent = get_indentation(stripped_line);
                for (i = 0; i < token_count; i++) {
                    macro = find_macro(&macro_table, tokens[i]);
                    if (macro) {
                        /* Expand macro */
                        for (j = 0; j < macro->line_count; j++) {
                            fprintf(output, "%s%s\n", base_indent, macro->lines[j]);
                        }
                        expanded = 1;
                        break;
                    }
                }
                if (!expanded) {
                    /* Write original line if not a macro */
                    fprintf(output, "%s", stripped_line);
                }
                free(base_indent);
            }
        }

        /* Clean up */
        for (i = 0; i < token_count; i++) {
            free(tokens[i]);
        }
        free(tokens);
        free(trimmed);
    }

    /* Check for unterminated macro */
    if (in_macro) {
        report_error(line_number, "Unterminated macro definition");
    }

    /* Final cleanup */
    free_macro_table(&macro_table);
    free(macro_name);
    free(line);
    fclose(input);
    fclose(output);
    free(input_filename);
    free(output_filename);
    free(base_filename);

    return get_error_count() > 0 ? 1 : 0;
}