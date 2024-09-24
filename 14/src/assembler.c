/***************************************************************
 * File: assembler.c
 * Description: Implements core assembler functions and data structures.
 ***************************************************************/

#define _POSIX_C_SOURCE 200809L
#include "assembler.h"
#include "helper_functions.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const char* OPCODE_STRINGS[NUM_OPCODES] = {
    "mov", "cmp", "add", "sub", "lea", "clr", "not", "inc",
    "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop"
};

/* create_symbol_table: Initializes and returns an empty symbol table */
SymbolTable create_symbol_table(void)
{
    SymbolTable table;
    table.symbols = NULL;
    table.count = 0;
    table.capacity = 0;
    return table;
}

/* free_symbol_table: Frees all memory associated with the given symbol table */
void free_symbol_table(SymbolTable* table)
{
    int i;
    for (i = 0; i < table->count; i++) {
        free(table->symbols[i].name);
    }
    free(table->symbols);
    table->count = 0;
    table->capacity = 0;
}

/* add_symbol: Adds a new symbol to the symbol table or updates an existing one */
void add_symbol(SymbolTable* table, const char* name, int value, bool is_data, bool is_code, bool is_entry, bool is_external)
{
    Symbol* existing;
    Symbol* new_symbol;

    existing = find_symbol(table, name);
    if (existing) {
        /* Update existing symbol */
        existing->value = value;
        existing->is_data |= is_data;
        existing->is_code |= is_code;
        existing->is_entry |= is_entry;
        existing->is_external |= is_external;
    } else {
        /* Add new symbol */
        if (table->count == table->capacity) {
            /* Grow the table if needed */
            table->capacity = table->capacity ? table->capacity * 2 : 1;
            table->symbols = (Symbol*)realloc(table->symbols, table->capacity * sizeof(Symbol));
        }
        
        new_symbol = &table->symbols[table->count++];
        new_symbol->name = strdup(name);
        new_symbol->value = value;
        new_symbol->is_data = is_data;
        new_symbol->is_code = is_code;
        new_symbol->is_entry = is_entry;
        new_symbol->is_external = is_external;
    }
}

/* find_symbol: Searches for a symbol in the symbol table */
Symbol* find_symbol(SymbolTable* table, const char* name)
{
    int i;
    for (i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL; /* Symbol not found */
}

/* init_memory_image: Initializes a new memory image structure */
void init_memory_image(MemoryImage* image)
{
    /* Initialize all fields to zero or NULL */
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

/* free_memory_image: Frees all memory associated with the given memory image */
void free_memory_image(MemoryImage* image)
{
    int i;
    free(image->code);
    free(image->data);
    for (i = 0; i < image->external_count; i++) {
        free(image->externals[i].symbol_name);
    }
    free(image->externals);
    init_memory_image(image); /* Reset to initial state */
}

/* add_to_memory_image: Adds a new word to the memory image */
void add_to_memory_image(MemoryImage* image, unsigned short value, AREType are, bool is_data)
{
    unsigned short** array;
    int* count;
    int* capacity;

    /* Determine which array (code or data) to use */
    array = is_data ? &image->data : &image->code;
    count = is_data ? &image->data_count : &image->code_count;
    capacity = is_data ? &image->data_capacity : &image->code_capacity;

    if (*count == *capacity) {
        /* Grow the array if needed */
        *capacity = *capacity ? *capacity * 2 : 1;
        *array = (unsigned short*)realloc(*array, *capacity * sizeof(unsigned short));
        if (!*array) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }

    /* Add the new word, combining value and ARE bits */
    (*array)[(*count)++] = (value & 0x7FFF) | (are & 0x7);
}

/* add_external_reference: Adds a new external reference to the memory image */
void add_external_reference(MemoryImage* memory_image, const char* symbol_name, int address)
{
    if (memory_image->external_count >= memory_image->external_capacity) {
        /* Grow the externals array if needed */
        memory_image->external_capacity = memory_image->external_capacity ? memory_image->external_capacity * 2 : 1;
        memory_image->externals = (ExternalReference*)realloc(memory_image->externals, memory_image->external_capacity * sizeof(ExternalReference));
        if (!memory_image->externals) {
            fprintf(stderr, "Memory allocation failed for external references\n");
            exit(1);
        }
    }
    /* Add the new external reference */
    memory_image->externals[memory_image->external_count].symbol_name = strdup(symbol_name);
    memory_image->externals[memory_image->external_count].address = address;
    memory_image->external_count++;
}

/* encode_instruction: Encodes an instruction into its binary representation */
unsigned short encode_instruction(int opcode, AddressingMode src_mode, AddressingMode dst_mode)
{
    /* Combine opcode and addressing modes into a single 15-bit word */
    return (opcode << 11) | (src_mode << 7) | (dst_mode << 3);
}

/* encode_operand: Encodes an operand into its binary representation */
unsigned short encode_operand(const char* operand, SymbolTable* symbol_table, AREType* are)
{
    int value;
    char* base;
    char* index;
    char* end_bracket;
    Symbol* base_symbol;
    int reg_num;
    Symbol* symbol;

    switch (encode_addressing_mode(operand)) {
        case IMMEDIATE:
            *are = ARE_ABSOLUTE;
            value = atoi(operand + 1); /* Skip the '#' character */
            return twos_complement(value);
        case REGISTER:
            *are = ARE_ABSOLUTE;
            return get_register_number(operand) << 3;
        case INDEX:
            *are = ARE_RELOCATABLE;
            base = strdup(operand);
            index = strchr(base, '[');
            *index = '\0';
            index++;
            end_bracket = strchr(index, ']');
            *end_bracket = '\0';
            
            base_symbol = find_symbol(symbol_table, base);
            reg_num = get_register_number(index);
            
            free(base);
            
            if (base_symbol && reg_num != -1) {
                return (base_symbol->value << 3) | reg_num;
            } else {
                report_error(0, "Invalid index addressing");
                return 0;
            }
        case DIRECT:
            if (operand[0] == '*') {
                /* Register indirect addressing */
                *are = ARE_ABSOLUTE;
                return get_register_number(operand + 1) << 3;
            } else {
                /* Direct symbol addressing */
                symbol = find_symbol(symbol_table, operand);
                if (symbol) {
                    *are = symbol->is_external ? ARE_EXTERNAL : ARE_RELOCATABLE;
                    return symbol->value;
                }
                report_error(0, "Symbol not found");
                return 0;
            }
    }
    return 0; /* Should never reach here */
}

/* get_opcode_type: Returns the type of the given opcode */
OpcodeType get_opcode_type(Opcode opcode)
{
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
            return OPCODE_NO_OPERANDS; /* Default case for safety */
    }
}

/* get_operand_words: Returns the number of words required to encode an operand */
int get_operand_words(AddressingMode mode)
{
    switch (mode) {
        case IMMEDIATE:
        case REGISTER:
            return 1; /* These modes require only one word */
        case DIRECT:
        case INDEX:
            return 2; /* These modes require two words */
        default:
            return 0; /* Invalid mode */
    }
}