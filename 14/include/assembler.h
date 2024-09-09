#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdbool.h>

#define MAX_LABEL_LENGTH 31
#define MAX_LINE_LENGTH 80
#define MEMORY_SIZE 4096
#define NUM_REGISTERS 8
#define NUM_OPCODES 16

typedef enum {
    MOV, CMP, ADD, SUB, LEA, CLR, NOT, INC, DEC, JMP, BNE, RED, PRN, JSR, RTS, STOP
} Opcode;

extern const char* OPCODE_STRINGS[NUM_OPCODES];

typedef enum {
    IMMEDIATE, DIRECT, INDEX, REGISTER
} AddressingMode;

typedef struct {
    char* name;
    int value;
    bool is_data;
    bool is_code;
    bool is_entry;
    bool is_external;
} Symbol;

typedef struct {
    Symbol* symbols;
    int count;
    int capacity;
} SymbolTable;

typedef struct {
    unsigned short* code;
    int code_count;
    int code_capacity;
    unsigned short* data;
    int data_count;
    int data_capacity;
} MemoryImage;

// Function prototypes for symbol table operations
SymbolTable create_symbol_table();
void free_symbol_table(SymbolTable* table);
void add_symbol(SymbolTable* table, const char* name, int value, bool is_data, bool is_code, bool is_entry, bool is_external);
Symbol* find_symbol(SymbolTable* table, const char* name);

// Utility functions
int get_opcode(const char* instruction);
int get_register_number(const char* reg);
int encode_addressing_mode(const char* operand);

#endif /* ASSEMBLER_H */