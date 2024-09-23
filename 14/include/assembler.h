#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdbool.h>

#define MAX_LABEL_LENGTH 31
#define NUM_REGISTERS 8
#define NUM_OPCODES 16
#define WORD_SIZE 15

typedef enum {
    MOV, CMP, ADD, SUB, LEA, CLR, NOT, INC, DEC, JMP, BNE, RED, PRN, JSR, RTS, STOP
} Opcode;

extern const char* OPCODE_STRINGS[NUM_OPCODES];

typedef enum {
    IMMEDIATE, DIRECT, INDEX, REGISTER
} AddressingMode;

typedef enum {
    ARE_ABSOLUTE = 4,
    ARE_RELOCATABLE = 2,
    ARE_EXTERNAL = 1
} AREType;

typedef enum {
    OPCODE_NO_OPERANDS,
    OPCODE_ONE_OPERAND,
    OPCODE_TWO_OPERANDS
} OpcodeType;

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
    char* symbol_name;
    int address;
} ExternalReference;

typedef struct {
    unsigned short* code;
    unsigned short* data;
    int code_count;
    int data_count;
    int code_capacity;
    int data_capacity;
    ExternalReference* externals;
    int external_count;
    int external_capacity;
} MemoryImage;

// Function prototypes for symbol table operations
SymbolTable create_symbol_table();
void free_symbol_table(SymbolTable* table);
void add_symbol(SymbolTable* table, const char* name, int value, bool is_data, bool is_code, bool is_entry, bool is_external);
Symbol* find_symbol(SymbolTable* table, const char* name);

// Function prototypes for memory image operations
void init_memory_image(MemoryImage* image);
void free_memory_image(MemoryImage* image);
void add_to_memory_image(MemoryImage* image, unsigned short value, AREType are, bool is_data);
void add_external_reference(MemoryImage* memory_image, const char* symbol_name, int address);

// Remaining utility functions
unsigned short encode_instruction(int opcode, AddressingMode src_mode, AddressingMode dst_mode);
unsigned short encode_operand(const char* operand, SymbolTable* symbol_table, AREType* are);

// New function prototypes
OpcodeType get_opcode_type(Opcode opcode);
int get_operand_words(AddressingMode mode);

#endif /* ASSEMBLER_H */