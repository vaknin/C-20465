/***************************************************************
 * File: second_pass.c
 * Description: Implements the second pass of the assembler.
 ***************************************************************/

#define _POSIX_C_SOURCE 200809L
#include "second_pass.h"
#include "assembler.h"
#include "helper_functions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* second_pass: Performs second pass, generating machine code and resolving symbols */
int second_pass(const char* filename, SymbolTable* symbol_table, MemoryImage* memory_image) {
    FILE* file;
    char* line = NULL;
    size_t len = 0;
    int read;
    int line_number = 0;
    int IC = 100;  /* Starting address for code segment */
    int i;

    file = fopen(filename, "r");
    if (!file) {
        report_error(0, "Error opening file");
        return 1;
    }

    while ((read = getline(&line, &len, file)) != -1) {
        char* ptr;
        char* token;
        line_number++;
        ptr = line;
        token = get_next_token(&ptr);
        if (!token) continue;  /* Skip empty lines */

        /* Skip label if present */
        if (is_label(token)) {
            free(token);
            token = get_next_token(&ptr);
        }

        if (!token) continue;

        if (token[0] == '.') {
            /* Handle directives (.data, .string) */
            if (strcmp(token, ".data") == 0) {
                char* num;
                while ((num = get_next_token(&ptr)) != NULL) {
                    int value = atoi(num);
                    add_to_memory_image(memory_image, twos_complement(value), ARE_ABSOLUTE, true);
                    free(num);
                }
            } else if (strcmp(token, ".string") == 0) {
                char* str = get_next_token(&ptr);
                if (str && str[0] == '"') {
                    int i;
                    for (i = 1; str[i] != '"' && str[i] != '\0'; i++) {
                        add_to_memory_image(memory_image, str[i], ARE_ABSOLUTE, true);
                    }
                    add_to_memory_image(memory_image, 0, ARE_ABSOLUTE, true);  /* Null terminator */
                } else {
                    report_error(line_number, "Invalid string directive");
                }
                free(str);
            }
            /* .extern and .entry are handled in the first pass */
        } else {
            /* Handle instructions */
            int opcode = get_opcode(token);
            char* src;
            char* dst;
            AddressingMode src_mode;
            AddressingMode dst_mode;
            unsigned short encoded;

            if (opcode == -1) {
                report_error(line_number, "Invalid instruction");
                free(token);
                continue;
            }

            src = get_next_token(&ptr);
            dst = get_next_token(&ptr);

            src_mode = src ? encode_addressing_mode(src) : 0;
            dst_mode = dst ? encode_addressing_mode(dst) : 0;

            encoded = encode_instruction(opcode, src_mode, dst_mode);
            add_to_memory_image(memory_image, encoded, ARE_ABSOLUTE, false);
            IC++;

            /* Handle source operand */
            if (src) {
                AREType are;
                unsigned short src_encoded = encode_operand(src, symbol_table, &are);
                add_to_memory_image(memory_image, src_encoded, are, false);
                IC++;

                if (src_mode == DIRECT && src[0] != '*') {
                    Symbol* symbol = find_symbol(symbol_table, src);
                    if (symbol) {
                        add_to_memory_image(memory_image, symbol->value, symbol->is_external ? ARE_EXTERNAL : ARE_RELOCATABLE, false);
                        if (symbol->is_external) {
                            add_external_reference(memory_image, symbol->name, IC);
                        }
                    } else {
                        report_error(line_number, "Symbol not found");
                    }
                    IC++;
                }
            }

            /* Handle destination operand */
            if (dst) {
                AREType are;
                unsigned short dst_encoded = encode_operand(dst, symbol_table, &are);
                add_to_memory_image(memory_image, dst_encoded, are, false);
                IC++;

                if (dst_mode == DIRECT && dst[0] != '*') {
                    Symbol* symbol = find_symbol(symbol_table, dst);
                    if (symbol) {
                        add_to_memory_image(memory_image, symbol->value, symbol->is_external ? ARE_EXTERNAL : ARE_RELOCATABLE, false);
                        if (symbol->is_external) {
                            add_external_reference(memory_image, symbol->name, IC);
                        }
                    } else {
                        report_error(line_number, "Symbol not found");
                    }
                    IC++;
                }
            }

            free(src);
            free(dst);
        }

        free(token);
    }

    /* Check for undefined symbols (excluding externals) */
    for (i = 0; i < symbol_table->count; i++) {
        if (!symbol_table->symbols[i].is_external && 
            (symbol_table->symbols[i].is_code || symbol_table->symbols[i].is_data) && 
            symbol_table->symbols[i].value == 0) {
            report_error(0, "Undefined symbol");
        }
    }

    free(line);
    fclose(file);
    return get_error_count() > 0 ? 1 : 0;
}

/* write_output_files: Generates .ob, .ent, and .ext output files */
void write_output_files(const char* filename, MemoryImage* memory_image, SymbolTable* symbol_table) {
    char ob_filename[256], ent_filename[256], ext_filename[256];
    FILE *ob_file, *ent_file, *ext_file;
    int i;

    sprintf(ob_filename, "%s.ob", filename);
    sprintf(ent_filename, "%s.ent", filename);
    sprintf(ext_filename, "%s.ext", filename);

    /* Write .ob file */
    ob_file = fopen(ob_filename, "w");
    if (ob_file) {
        fprintf(ob_file, "%d %d\n", memory_image->code_count, memory_image->data_count);
        
        /* Write code words */
        for (i = 0; i < memory_image->code_count; i++) {
            fprintf(ob_file, "%04d %05o\n", 100 + i, memory_image->code[i] & 0x7FFF);
        }
        
        /* Write data words */
        for (i = 0; i < memory_image->data_count; i++) {
            fprintf(ob_file, "%04d %05o\n", 100 + memory_image->code_count + i, memory_image->data[i] & 0x7FFF);
        }
        
        fclose(ob_file);
    } else {
        report_error(0, "Error opening .ob file for writing");
    }

    /* Write .ent file */
    ent_file = fopen(ent_filename, "w");
    if (ent_file) {
        for (i = 0; i < symbol_table->count; i++) {
            if (symbol_table->symbols[i].is_entry) {
                fprintf(ent_file, "%s %04d\n", symbol_table->symbols[i].name, symbol_table->symbols[i].value);
            }
        }
        fclose(ent_file);
    } else {
        report_error(0, "Error opening .ent file for writing");
    }

    /* Write .ext file */
    ext_file = fopen(ext_filename, "w");
    if (ext_file) {
        for (i = 0; i < memory_image->external_count; i++) {
            fprintf(ext_file, "%s %04d\n", memory_image->externals[i].symbol_name, memory_image->externals[i].address);
        }
        fclose(ext_file);
    } else {
        report_error(0, "Error opening .ext file for writing");
    }
}