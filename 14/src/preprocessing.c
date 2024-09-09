#define _GNU_SOURCE
#include "preprocessing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

MacroTable create_macro_table(void) {
    return (MacroTable) {NULL, 0, 0};
}

void add_macro(MacroTable* table, const char* name, char** lines, int line_count) {
    if (table->count == table->capacity) {
        table->capacity = table->capacity ? table->capacity * 2 : 1;
        table->items = realloc(table->items, table->capacity * sizeof(Macro));
    }
    Macro* macro = &table->items[table->count++];
    macro->name = strdup(name);
    macro->lines = malloc(line_count * sizeof(char*));
    macro->line_count = line_count;
    for (int i = 0; i < line_count; i++) {
        macro->lines[i] = strdup(lines[i]);
    }
}

Macro* find_macro(MacroTable* table, const char* name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->items[i].name, name) == 0) {
            return &table->items[i];
        }
    }
    return NULL;
}

void free_macro_table(MacroTable* table) {
    for (int i = 0; i < table->count; i++) {
        free(table->items[i].name);
        for (int j = 0; j < table->items[i].line_count; j++) {
            free(table->items[i].lines[j]);
        }
        free(table->items[i].lines);
    }
    free(table->items);
    *table = (MacroTable) {NULL, 0, 0};
}

char* trim_end(char* str) {
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

char* get_first_token(const char* str) {
    while (isspace((unsigned char)*str)) str++;
    const char* end = str;
    while (*end && !isspace((unsigned char)*end)) end++;
    return strndup(str, end - str);
}

char* get_indentation(const char* str) {
    const char* end = str;
    while (*end && isspace((unsigned char)*end)) end++;
    return strndup(str, end - str);
}

char** tokenize_line(const char* line, int* token_count) {
    char** tokens = NULL;
    *token_count = 0;
    int capacity = 0;
    const char* start = line;

    while (*start) {
        while (isspace((unsigned char)*start)) start++;
        if (*start == '\0') break;

        const char* end = start;
        while (*end && !isspace((unsigned char)*end)) end++;

        if (*token_count == capacity) {
            capacity = capacity ? capacity * 2 : 1;
            tokens = realloc(tokens, capacity * sizeof(char*));
        }
        tokens[(*token_count)++] = strndup(start, end - start);
        start = end;
    }

    return tokens;
}

char* trim_start(char* str) {
    while (isspace((unsigned char)*str)) str++;
    return str;
}

int preprocess_file(const char* filename) {
    char *input_filename, *output_filename, *base_filename;
    const char* extension = strrchr(filename, '.');
    
    if (extension && strcmp(extension, ".as") == 0) {
        input_filename = strdup(filename);
        base_filename = strndup(filename, extension - filename);
    } else {
        input_filename = malloc(strlen(filename) + 4);
        sprintf(input_filename, "%s.as", filename);
        base_filename = strdup(filename);
    }
    
    output_filename = malloc(strlen(base_filename) + 4);
    sprintf(output_filename, "%s.am", base_filename);

    FILE* input = fopen(input_filename, "r");
    FILE* output = fopen(output_filename, "w");
    if (!input || !output) {
        fprintf(stderr, "Error opening files: %s or %s.\n", input_filename, output_filename);
        free(input_filename);
        free(output_filename);
        free(base_filename);
        return 1;
    }

    MacroTable macro_table = create_macro_table();
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char* macro_name = NULL;
    char** macro_lines = NULL;
    int macro_line_count = 0, macro_lines_capacity = 0;
    int in_macro = 0;

    while ((read = getline(&line, &len, input)) != -1) {
        char* trimmed = trim_end(strdup(line));
        int token_count;
        char** tokens = tokenize_line(trimmed, &token_count);

        if (token_count > 0) {
            if (strcmp(tokens[0], "macr") == 0 && token_count > 1) {
                free(macro_name);
                macro_name = strdup(tokens[1]);
                in_macro = 1;
                macro_line_count = 0;
            } else if (strcmp(tokens[0], "endmacr") == 0) {
                add_macro(&macro_table, macro_name, macro_lines, macro_line_count);
                in_macro = 0;
                for (int i = 0; i < macro_line_count; i++) {
                    free(macro_lines[i]);
                }
                free(macro_lines);
                macro_lines = NULL;
                macro_line_count = macro_lines_capacity = 0;
            } else if (in_macro) {
                if (macro_line_count == macro_lines_capacity) {
                    macro_lines_capacity = macro_lines_capacity ? macro_lines_capacity * 2 : 1;
                    macro_lines = realloc(macro_lines, macro_lines_capacity * sizeof(char*));
                }
                macro_lines[macro_line_count++] = strdup(trim_start(trimmed));
            } else {
                int expanded = 0;
                char* base_indent = get_indentation(line);
                for (int i = 0; i < token_count; i++) {
                    Macro* macro = find_macro(&macro_table, tokens[i]);
                    if (macro) {
                        for (int j = 0; j < macro->line_count; j++) {
                            fprintf(output, "%s%s\n", base_indent, macro->lines[j]);
                        }
                        expanded = 1;
                        break;
                    }
                }
                if (!expanded) {
                    fprintf(output, "%s", line);
                }
                free(base_indent);
            }
        }

        for (int i = 0; i < token_count; i++) {
            free(tokens[i]);
        }
        free(tokens);
        free(trimmed);
    }

    free_macro_table(&macro_table);
    free(macro_name);
    free(line);
    fclose(input);
    fclose(output);
    free(input_filename);
    free(output_filename);
    free(base_filename);
    return 0;
}