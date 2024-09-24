/***************************************************************
 * File: assembler.h
 * Description: Defines data structures and function prototypes 
 *              for the assembler program.
 ***************************************************************/

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "helper_functions.h"

#define MAX_LABEL_LENGTH 31
#define NUM_REGISTERS 8
#define NUM_OPCODES 16
#define WORD_SIZE 15

/* Opcode: Enumeration of supported assembly instructions */
typedef enum {
    MOV, CMP, ADD, SUB, LEA, CLR, NOT, INC, DEC, JMP, BNE, RED, PRN, JSR, RTS, STOP
} Opcode;

extern const char* OPCODE_STRINGS[NUM_OPCODES];

/* AddressingMode: Enumeration of supported addressing modes */
typedef enum {
    IMMEDIATE, DIRECT, INDEX, REGISTER
} AddressingMode;

/* AREType: Enumeration of Absolute, Relocatable, and External types */
typedef enum {
    ARE_ABSOLUTE = 4,
    ARE_RELOCATABLE = 2,
    ARE_EXTERNAL = 1
} AREType;

/* OpcodeType: Enumeration of opcode types based on operand count */
typedef enum {
    OPCODE_NO_OPERANDS,
    OPCODE_ONE_OPERAND,
    OPCODE_TWO_OPERANDS
} OpcodeType;

/* Symbol: Structure representing a symbol in the assembly code */
typedef struct {
    char* name;
    int value;
    int is_data;
    int is_code;
    int is_entry;
    int is_external;
} Symbol;

/* SymbolTable: Structure representing the symbol table */
typedef struct {
    Symbol* symbols;
    int count;
    int capacity;
} SymbolTable;

/* ExternalReference: Structure representing an external symbol reference */
typedef struct {
    char* symbol_name;
    int address;
} ExternalReference;

/* MemoryImage: Structure representing the assembled memory image */
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

/* Function prototypes for symbol table operations */

/* create_symbol_table: Initializes and returns a new empty symbol table. */
SymbolTable create_symbol_table(void);

/* free_symbol_table: Frees all memory associated with the given symbol table. */
void free_symbol_table(SymbolTable* table);

/* add_symbol: Adds a new symbol to the symbol table or updates an existing one. */
void add_symbol(SymbolTable* table, const char* name, int value, int is_data, int is_code, int is_entry, int is_external);

/* find_symbol: Searches for a symbol in the symbol table and returns a pointer to it if found, or NULL if not found. */
Symbol* find_symbol(SymbolTable* table, const char* name);

/* Function prototypes for memory image operations */

/* init_memory_image: Initializes a new memory image structure. */
void init_memory_image(MemoryImage* image);

/* free_memory_image: Frees all memory associated with the given memory image. */
void free_memory_image(MemoryImage* image);

/* add_to_memory_image: Adds a new word to the memory image (either code or data section). */
void add_to_memory_image(MemoryImage* image, unsigned short value, AREType are, int is_data);

/* add_external_reference: Adds a new external reference to the memory image. */
void add_external_reference(MemoryImage* memory_image, const char* symbol_name, int address);

/* Utility function prototypes */

/* encode_instruction: Encodes an instruction into its binary representation. */
unsigned short encode_instruction(int opcode, AddressingMode src_mode, AddressingMode dst_mode);

/* encode_operand: Encodes an operand into its binary representation. */
unsigned short encode_operand(const char* operand, SymbolTable* symbol_table, AREType* are);

/* get_opcode_type: Returns the type of the given opcode (no operands, one operand, or two operands). */
OpcodeType get_opcode_type(Opcode opcode);

/* get_operand_words: Returns the number of words required to encode an operand with the given addressing mode. */
int get_operand_words(AddressingMode mode);

#endif /* ASSEMBLER_H */