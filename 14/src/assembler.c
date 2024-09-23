#include "assembler.h"
#include "helper_functions.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
    Symbol* existing = find_symbol(table, name);
    if (existing) {
        existing->value = value;
        existing->is_data |= is_data;
        existing->is_code |= is_code;
        existing->is_entry |= is_entry;
        existing->is_external |= is_external;
    } else {
        if (table->count == table->capacity) {
            table->capacity = table->capacity ? table->capacity * 2 : 1;
            table->symbols = realloc(table->symbols, table->capacity * sizeof(Symbol));
        }
        
        Symbol* new_symbol = &table->symbols[table->count++];
        new_symbol->name = strdup(name);
        new_symbol->value = value;
        new_symbol->is_data = is_data;
        new_symbol->is_code = is_code;
        new_symbol->is_entry = is_entry;
        new_symbol->is_external = is_external;
    }
}

Symbol* find_symbol(SymbolTable* table, const char* name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

void init_memory_image(MemoryImage* image) {
    image->code = NULL;
    image->data = NULL;
    image->code_count = 0;
    image->data_count = 0;
    image->code_capacity = 0;
    image->data_capacity = 0;
    image->externals = NULL;
    image->external_count = 0;
    image->external_capacity = 0;
}

void free_memory_image(MemoryImage* image) {
    free(image->code);
    free(image->data);
    for (int i = 0; i < image->external_count; i++) {
        free(image->externals[i].symbol_name);
    }
    free(image->externals);
    init_memory_image(image);
}

void add_to_memory_image(MemoryImage* image, unsigned short value, AREType are, bool is_data) {
    unsigned short** array = is_data ? &image->data : &image->code;
    int* count = is_data ? &image->data_count : &image->code_count;
    int* capacity = is_data ? &image->data_capacity : &image->code_capacity;

    if (*count == *capacity) {
        *capacity = *capacity ? *capacity * 2 : 1;
        *array = realloc(*array, *capacity * sizeof(unsigned short));
        if (!*array) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }

    (*array)[(*count)++] = (value & 0x7FFF) | (are & 0x7);
}

void add_external_reference(MemoryImage* memory_image, const char* symbol_name, int address) {
    if (memory_image->external_count >= memory_image->external_capacity) {
        memory_image->external_capacity = memory_image->external_capacity ? memory_image->external_capacity * 2 : 1;
        memory_image->externals = realloc(memory_image->externals, memory_image->external_capacity * sizeof(ExternalReference));
        if (!memory_image->externals) {
            fprintf(stderr, "Memory allocation failed for external references\n");
            exit(1);
        }
    }
    memory_image->externals[memory_image->external_count].symbol_name = strdup(symbol_name);
    memory_image->externals[memory_image->external_count].address = address;
    memory_image->external_count++;
}

unsigned short encode_instruction(int opcode, AddressingMode src_mode, AddressingMode dst_mode) {
    return (opcode << 11) | (src_mode << 7) | (dst_mode << 3);
}

unsigned short encode_operand(const char* operand, SymbolTable* symbol_table, AREType* are) {
    int value;
    switch (encode_addressing_mode(operand)) {
        case IMMEDIATE:
            *are = ARE_ABSOLUTE;
            value = atoi(operand + 1);
            return twos_complement(value);
        case REGISTER:
            *are = ARE_ABSOLUTE;
            return get_register_number(operand) << 3;
        case INDEX: {
            *are = ARE_RELOCATABLE;
            char* base = strdup(operand);
            char* index = strchr(base, '[');
            *index = '\0';
            index++;
            char* end_bracket = strchr(index, ']');
            *end_bracket = '\0';
            
            Symbol* base_symbol = find_symbol(symbol_table, base);
            int reg_num = get_register_number(index);
            
            free(base);
            
            if (base_symbol && reg_num != -1) {
                return (base_symbol->value << 3) | reg_num;
            } else {
                report_error(0, "Invalid index addressing");
                return 0;
            }
        }
        case DIRECT:
            if (operand[0] == '*') {
                *are = ARE_ABSOLUTE;
                return get_register_number(operand + 1) << 3;
            } else {
                Symbol* symbol = find_symbol(symbol_table, operand);
                if (symbol) {
                    *are = symbol->is_external ? ARE_EXTERNAL : ARE_RELOCATABLE;
                    return symbol->value;
                }
                report_error(0, "Symbol not found");
                return 0;
            }
    }
    return 0;
}

OpcodeType get_opcode_type(Opcode opcode) {
    switch (opcode) {
        case MOV:
        case CMP:
        case ADD:
        case SUB:
        case LEA:
            return OPCODE_TWO_OPERANDS;
        case CLR:
        case NOT:
        case INC:
        case DEC:
        case JMP:
        case BNE:
        case RED:
        case PRN:
        case JSR:
            return OPCODE_ONE_OPERAND;
        case RTS:
        case STOP:
            return OPCODE_NO_OPERANDS;
        default:
            return OPCODE_NO_OPERANDS;
    }
}

int get_operand_words(AddressingMode mode) {
    switch (mode) {
        case IMMEDIATE:
        case REGISTER:
            return 1;
        case DIRECT:
        case INDEX:
            return 2;
        default:
            return 0;
    }
}