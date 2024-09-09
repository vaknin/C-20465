#include "assembler.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

const char* OPCODE_STRINGS[NUM_OPCODES] = {
    "mov", "cmp", "add", "sub", "lea", "clr", "not", "inc",
    "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop"
};

SymbolTable create_symbol_table() {
    SymbolTable table;
    table.symbols = NULL;
    table.count = 0;
    table.capacity = 0;
    return table;
}

void free_symbol_table(SymbolTable* table) {
    for (int i = 0; i < table->count; i++) {
        free(table->symbols[i].name);
    }
    free(table->symbols);
    table->count = 0;
    table->capacity = 0;
}

void add_symbol(SymbolTable* table, const char* name, int value, bool is_data, bool is_code, bool is_entry, bool is_external) {
    if (table->count == table->capacity) {
        table->capacity = table->capacity ? table->capacity * 2 : 1;
        table->symbols = realloc(table->symbols, table->capacity * sizeof(Symbol));
    }
    
    Symbol* symbol = &table->symbols[table->count++];
    symbol->name = strdup(name);
    symbol->value = value;
    symbol->is_data = is_data;
    symbol->is_code = is_code;
    symbol->is_entry = is_entry;
    symbol->is_external = is_external;
}

Symbol* find_symbol(SymbolTable* table, const char* name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

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

int get_opcode(const char* instruction) {
    for (int i = 0; i < NUM_OPCODES; i++) {
        if (strcmp(instruction, OPCODE_STRINGS[i]) == 0) {
            return i;
        }
    }
    return -1;
}

int get_register_number(const char* reg) {
    if (reg[0] == 'r' && isdigit(reg[1])) {
        int num = atoi(reg + 1);
        if (num >= 0 && num <= 7) {
            return num;
        }
    }
    return -1;
}

int encode_addressing_mode(const char* operand) {
    if (operand[0] == '#') return IMMEDIATE;
    if (operand[0] == 'r') return REGISTER;
    if (strchr(operand, '[') && strchr(operand, ']')) return INDEX;
    return DIRECT;
}

void init_memory_image(MemoryImage* image) {
    image->code = NULL;
    image->data = NULL;
    image->code_count = 0;
    image->data_count = 0;
    image->code_capacity = 0;
    image->data_capacity = 0;
}

void free_memory_image(MemoryImage* image) {
    free(image->code);
    free(image->data);
    image->code = NULL;
    image->data = NULL;
    image->code_count = 0;
    image->data_count = 0;
    image->code_capacity = 0;
    image->data_capacity = 0;
}