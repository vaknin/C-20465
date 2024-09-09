#include "second_pass.h"
#include "assembler.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define INITIAL_CAPACITY 100

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

static unsigned short encode_instruction(const char* instruction, const char* src, const char* dst) {
    unsigned short encoded = 0;
    int opcode = get_opcode(instruction);
    
    encoded |= (opcode & 0xF) << 11;  // Opcode in bits 11-14
    
    if (src) {
        int src_mode = encode_addressing_mode(src);
        encoded |= (src_mode & 0xF) << 7;  // Source addressing mode in bits 7-10
    }
    
    if (dst) {
        int dst_mode = encode_addressing_mode(dst);
        encoded |= (dst_mode & 0xF) << 3;  // Destination addressing mode in bits 3-6
    }
    
    return encoded;
}

static unsigned short encode_operand(const char* operand, SymbolTable* symbol_table) {
    if (operand[0] == '#') {
        return (unsigned short)atoi(operand + 1);
    } else if (operand[0] == 'r') {
        return (unsigned short)get_register_number(operand);
    } else {
        Symbol* symbol = find_symbol(symbol_table, operand);
        if (symbol) {
            return symbol->value;
        }
    }
    return 0;  // Error case, should be handled
}

static void add_to_memory_image(MemoryImage* image, unsigned short value, bool is_data) {
    unsigned short** array = is_data ? &image->data : &image->code;
    int* count = is_data ? &image->data_count : &image->code_count;
    int* capacity = is_data ? &image->data_capacity : &image->code_capacity;

    if (*count == *capacity) {
        *capacity *= 2;
        *array = realloc(*array, *capacity * sizeof(unsigned short));
    }

    (*array)[(*count)++] = value;
}

int second_pass(const char* filename, SymbolTable* symbol_table, MemoryImage* memory_image) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file: %s\n", filename);
        return 1;
    }

    char line[MAX_LINE_LENGTH];

    memory_image->code = malloc(INITIAL_CAPACITY * sizeof(unsigned short));
    memory_image->data = malloc(INITIAL_CAPACITY * sizeof(unsigned short));
    memory_image->code_capacity = memory_image->data_capacity = INITIAL_CAPACITY;
    memory_image->code_count = memory_image->data_count = 0;

    while (fgets(line, sizeof(line), file)) {
        char* ptr = line;
        char* token = get_next_token(&ptr);
        if (!token) continue;  // Skip empty lines

        // Skip label if present
        if (token[strlen(token) - 1] == ':') {
            free(token);
            token = get_next_token(&ptr);
        }

        if (!token) continue;

        if (token[0] == '.') {
            // Handle directives (.data, .string, .entry, .extern)
            if (strcmp(token, ".data") == 0) {
                char* num;
                while ((num = get_next_token(&ptr)) != NULL) {
                    add_to_memory_image(memory_image, (unsigned short)atoi(num), true);
                    free(num);
                }
            } else if (strcmp(token, ".string") == 0) {
                char* str = get_next_token(&ptr);
                if (str && str[0] == '"') {
                    for (int i = 1; str[i] != '"' && str[i] != '\0'; i++) {
                        add_to_memory_image(memory_image, (unsigned short)str[i], true);
                    }
                    add_to_memory_image(memory_image, 0, true);  // Null terminator
                }
                free(str);
            } else if (strcmp(token, ".entry") == 0) {
                char* symbol_name = get_next_token(&ptr);
                Symbol* symbol = find_symbol(symbol_table, symbol_name);
                if (symbol) {
                    symbol->is_entry = true;
                }
                free(symbol_name);
            }
        } else {
            // Handle instructions
            char* instruction = token;
            char* src = get_next_token(&ptr);
            char* dst = get_next_token(&ptr);

            unsigned short encoded = encode_instruction(instruction, src, dst);
            add_to_memory_image(memory_image, encoded, false);

            if (src) {
                unsigned short src_encoded = encode_operand(src, symbol_table);
                add_to_memory_image(memory_image, src_encoded, false);
            }

            if (dst) {
                unsigned short dst_encoded = encode_operand(dst, symbol_table);
                add_to_memory_image(memory_image, dst_encoded, false);
            }

            free(src);
            free(dst);
        }

        free(token);
    }

    fclose(file);
    return 0;
}

void write_output_files(const char* filename, MemoryImage* memory_image, SymbolTable* symbol_table) {
    char* ob_filename = malloc(strlen(filename) + 4);
    char* ent_filename = malloc(strlen(filename) + 5);
    char* ext_filename = malloc(strlen(filename) + 5);

    sprintf(ob_filename, "%s.ob", filename);
    sprintf(ent_filename, "%s.ent", filename);
    sprintf(ext_filename, "%s.ext", filename);

    // Write .ob file
    FILE* ob_file = fopen(ob_filename, "w");
    if (ob_file) {
        fprintf(ob_file, "%d %d\n", memory_image->code_count, memory_image->data_count);
        for (int i = 0; i < memory_image->code_count; i++) {
            fprintf(ob_file, "%04d %05o\n", 100 + i, memory_image->code[i]);
        }
        for (int i = 0; i < memory_image->data_count; i++) {
            fprintf(ob_file, "%04d %05o\n", 100 + memory_image->code_count + i, memory_image->data[i]);
        }
        fclose(ob_file);
    }

    // Write .ent file only if there are entry symbols
    bool has_entries = false;
    for (int i = 0; i < symbol_table->count; i++) {
        if (symbol_table->symbols[i].is_entry) {
            has_entries = true;
            break;
        }
    }
    
    if (has_entries) {
        FILE* ent_file = fopen(ent_filename, "w");
        if (ent_file) {
            for (int i = 0; i < symbol_table->count; i++) {
                if (symbol_table->symbols[i].is_entry) {
                    fprintf(ent_file, "%s %04d\n", symbol_table->symbols[i].name, symbol_table->symbols[i].value);
                }
            }
            fclose(ent_file);
        }
    }

    // Write .ext file only if there are external symbols
    bool has_externals = false;
    for (int i = 0; i < symbol_table->count; i++) {
        if (symbol_table->symbols[i].is_external) {
            has_externals = true;
            break;
        }
    }
    
    if (has_externals) {
        FILE* ext_file = fopen(ext_filename, "w");
        if (ext_file) {
            for (int i = 0; i < symbol_table->count; i++) {
                if (symbol_table->symbols[i].is_external) {
                    fprintf(ext_file, "%s %04d\n", symbol_table->symbols[i].name, symbol_table->symbols[i].value);
                }
            }
            fclose(ext_file);
        }
    }

    free(ob_filename);
    free(ent_filename);
    free(ext_filename);
}