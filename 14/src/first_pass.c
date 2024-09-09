#include "first_pass.h"
#include "assembler.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static int DC = 0;
static int IC = 0;

static char* skip_whitespace(char* str) {
    while (*str && isspace(*str)) str++;
    return str;
}

static char* get_next_token(char** str) {
    *str = skip_whitespace(*str);
    if (**str == '\0') return NULL;

    char* start = *str;
    if (**str == '"') {
        (*str)++;
        while (**str && **str != '"') (*str)++;
        if (**str == '"') (*str)++;
    } else {
        while (**str && !isspace(**str) && **str != ',') (*str)++;
    }
    
    if (*str != start) {
        char* token = strndup(start, *str - start);
        *str = skip_whitespace(*str);
        if (**str == ',') (*str)++;
        return token;
    }
    return NULL;
}

static bool is_label(const char* token) {
    return token[strlen(token) - 1] == ':';
}

static bool is_data_directive(const char* token) {
    return strcmp(token, ".data") == 0 || strcmp(token, ".string") == 0;
}

static bool is_extern_entry_directive(const char* token) {
    return strcmp(token, ".extern") == 0 || strcmp(token, ".entry") == 0;
}

static int count_data_words(const char* line) {
    int count = 0;
    const char* p = line;
    while (*p) {
        if (*p == '"') {  // String
            p++;
            while (*p && *p != '"') {
                count++;
                p++;
            }
            count++; // For null terminator
        } else if (isdigit(*p) || *p == '-' || *p == '+') {
            count++;
            while (isdigit(*p)) p++;
        }
        p++;
    }
    return count;
}

int get_instruction_length(const char* instruction) {
    char* ptr = (char*)instruction;
    char* opcode = get_next_token(&ptr);
    char* src = get_next_token(&ptr);
    char* dst = get_next_token(&ptr);
    
    int length = 1;  // First word always present
    
    if (src) {
        if (src[0] == '#' || src[0] == 'r') length++;
        else length += 2;  // Direct or index addressing
    }
    
    if (dst && strcmp(opcode, "rts") != 0 && strcmp(opcode, "stop") != 0) {
        if (dst[0] == '#' || dst[0] == 'r') length++;
        else length += 2;  // Direct or index addressing
    }
    
    free(opcode);
    free(src);
    free(dst);
    
    return length;
}

int first_pass(const char* filename, SymbolTable* symbol_table) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file: %s\n", filename);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    char* label = malloc(MAX_LABEL_LENGTH + 1);

    while (fgets(line, sizeof(line), file)) {
        char* ptr = line;
        char* token = get_next_token(&ptr);
        if (!token) continue;  // Skip empty lines

        bool label_found = false;
        if (is_label(token)) {
            strncpy(label, token, MAX_LABEL_LENGTH);
            label[strlen(label) - 1] = '\0';  // Remove colon
            label_found = true;
            free(token);
            token = get_next_token(&ptr);
        }

        if (!token) continue;

        if (is_data_directive(token)) {
            if (label_found) {
                add_symbol(symbol_table, label, DC, true, false, false, false);
            }
            DC += count_data_words(ptr);
        } else if (is_extern_entry_directive(token)) {
            char* symbol_name = get_next_token(&ptr);
            if (symbol_name) {
                if (strcmp(token, ".extern") == 0) {
                    add_symbol(symbol_table, symbol_name, 0, false, false, false, true);
                }
                free(symbol_name);
            }
        } else {
            // Instruction
            if (label_found) {
                add_symbol(symbol_table, label, IC + 100, false, true, false, false);
            }
            IC += get_instruction_length(ptr);
        }

        free(token);
    }

    // Update data symbols
    for (int i = 0; i < symbol_table->count; i++) {
        if (symbol_table->symbols[i].is_data) {
            symbol_table->symbols[i].value += IC + 100;
        }
    }

    free(label);
    fclose(file);
    return 0;
}