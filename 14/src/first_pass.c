/***************************************************************
 * File: first_pass.c
 * Description: Implements the first pass of the assembler.
 ***************************************************************/

#define _POSIX_C_SOURCE 200809L
#include "first_pass.h"
#include "assembler.h"
#include "helper_functions.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* Start at 100 as per the specifications */
static int IC = 100;
static int DC = 0;

/* get_instruction_length: Calculates the length of an instruction */
int get_instruction_length(const char* instruction)
{
    char* instr_copy;
    char* original_ptr;
    char* token;
    Opcode opcode;
    int length;
    OpcodeType opcode_type;
    char* src;
    char* dst;
    AddressingMode src_mode, dst_mode;

    instr_copy = strdup(instruction);
    original_ptr = instr_copy;
    token = strtok(instr_copy, " \t");

    if (!token) {
        free(original_ptr);
        return 0;
    }

    /* Skip label if present */
    if (token[strlen(token) - 1] == ':') {
        token = strtok(NULL, " \t");
        if (!token) {
            free(original_ptr);
            return 0;
        }
    }

    /* Get opcode */
    opcode = get_opcode(token);
    if (opcode == -1) {
        free(original_ptr);
        return 0;
    }

    length = 1;  /* First word always present for opcode and addressing modes */

    /* Get opcode type */
    opcode_type = get_opcode_type(opcode);

    /* Parse operands */
    src = NULL;
    dst = NULL;

    switch (opcode_type) {
        case OPCODE_NO_OPERANDS:
            break;
        case OPCODE_ONE_OPERAND:
            dst = strtok(NULL, ",");
            if (dst) {
                dst_mode = encode_addressing_mode(dst);
                length += get_operand_words(dst_mode);
            }
            break;
        case OPCODE_TWO_OPERANDS:
            src = strtok(NULL, ",");
            dst = strtok(NULL, ",");
            if (src) {
                src_mode = encode_addressing_mode(src);
                length += get_operand_words(src_mode);
            }
            if (dst) {
                dst_mode = encode_addressing_mode(dst);
                length += get_operand_words(dst_mode);
            }
            break;
    }

    free(original_ptr);
    return length;
}

/* first_pass: Performs the first pass of the assembler */
int first_pass(const char* filename, SymbolTable* symbol_table)
{
    FILE* file;
    char* line = NULL;
    size_t len = 0;
    int read;
    int line_number = 0;
    char* stripped_line;
    char* ptr;
    char* token;
    char* label;
    Symbol* existing;
    int data_words;
    char* symbol_name;
    int instruction_length;
    int i;

    file = fopen(filename, "r");
    if (!file) {
        report_error(0, "Error opening file");
        return 1;
    }

    /* Reset IC and DC for each file */
    IC = 100;
    DC = 0;

    while ((read = getline(&line, &len, file)) != -1) {
        line_number++;
        stripped_line = strip_comments(line);
        ptr = stripped_line;
        token = get_next_token(&ptr);
        if (!token) continue;  /* Skip empty lines */

        label = NULL;
        if (is_label(token)) {
            label = strndup(token, strlen(token) - 1);  /* Remove colon */
            if (!is_valid_label(label)) {
                report_error(line_number, "Invalid label");
                free(label);
                free(token);
                continue;
            }
            free(token);
            token = get_next_token(&ptr);
        }

        if (!token) {
            free(label);
            continue;
        }

        if (is_data_directive(token)) {
            if (label) {
                existing = find_symbol(symbol_table, label);
                if (existing && (existing->is_data || existing->is_code)) {
                    report_error(line_number, "Duplicate symbol definition");
                } else {
                    add_symbol(symbol_table, label, DC, true, false, false, false);
                }
            }
            data_words = count_data_words(ptr);
            if (data_words == 0) {
                report_error(line_number, "Invalid data directive");
            }
            DC += data_words;
        } else if (is_extern_entry_directive(token)) {
            symbol_name = get_next_token(&ptr);
            if (symbol_name) {
                if (strcmp(token, ".extern") == 0) {
                    existing = find_symbol(symbol_table, symbol_name);
                    if (existing && !existing->is_external) {
                        report_error(line_number, "Symbol already defined locally");
                    } else {
                        add_symbol(symbol_table, symbol_name, 0, false, false, false, true);
                    }
                } else {  /* .entry */
                    existing = find_symbol(symbol_table, symbol_name);
                    if (existing) {
                        existing->is_entry = true;
                    } else {
                        add_symbol(symbol_table, symbol_name, 0, false, false, true, false);
                    }
                }
                free(symbol_name);
            } else {
                report_error(line_number, "Missing symbol name for extern/entry directive");
            }
        } else {
            /* Instruction */
            if (label) {
                existing = find_symbol(symbol_table, label);
                if (existing && (existing->is_data || existing->is_code)) {
                    report_error(line_number, "Duplicate symbol definition");
                } else {
                    add_symbol(symbol_table, label, IC, false, true, false, false);
                }
            }
            
            instruction_length = get_instruction_length(stripped_line);
            
            if (instruction_length > 0) {
                IC += instruction_length;
            } else {
                report_error(line_number, "Invalid instruction");
            }
        }

        free(token);
        free(label);
    }

    /* Update data symbol addresses */
    for (i = 0; i < symbol_table->count; i++) {
        if (symbol_table->symbols[i].is_data) {
            symbol_table->symbols[i].value += IC - 100;  /* Adjust by IC - 100 to get the correct offset */
        }
    }

    free(line);
    fclose(file);
    return get_error_count() > 0 ? 1 : 0;
}