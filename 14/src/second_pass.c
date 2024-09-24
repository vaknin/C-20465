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
    int error_occurred = 0;

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
                    add_to_memory_image(memory_image, (unsigned short)value, ARE_ABSOLUTE, 1);
                    free(num);
                }
            } else if (strcmp(token, ".string") == 0) {
                char* str = get_next_token(&ptr);
                if (str && str[0] == '"') {
                    int i;
                    for (i = 1; str[i] != '"' && str[i] != '\0'; i++) {
                        add_to_memory_image(memory_image, (unsigned short)str[i], ARE_ABSOLUTE, 1);
                    }
                    add_to_memory_image(memory_image, 0, ARE_ABSOLUTE, 1);  /* Null terminator */
                } else {
                    report_error(line_number, "Invalid string directive");
                    error_occurred = 1;
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
                error_occurred = 1;
                free(token);
                continue;
            }

            src = get_next_token(&ptr);
            dst = get_next_token(&ptr);

            src_mode = src ? encode_addressing_mode(src) : 0;
            dst_mode = dst ? encode_addressing_mode(dst) : 0;

            encoded = encode_instruction(opcode, src_mode, dst_mode);
            add_to_memory_image(memory_image, encoded, ARE_ABSOLUTE, 0);
            IC++;

            /* Handle source operand */
            if (src) {
                AREType are;
                unsigned short src_encoded = encode_operand(src, symbol_table, &are);
                add_to_memory_image(memory_image, src_encoded, are, 0);
                IC++;

                if (src_mode == DIRECT && src[0] != '*') {
                    Symbol* symbol = find_symbol(symbol_table, src);
                    if (symbol) {
                        add_to_memory_image(memory_image, (unsigned short)symbol->value, 
                                            symbol->is_external ? ARE_EXTERNAL : ARE_RELOCATABLE, 0);
                        if (symbol->is_external) {
                            add_external_reference(memory_image, symbol->name, IC);
                        }
                    } else {
                        report_error(line_number, "Symbol not found");
                        error_occurred = 1;
                    }
                    IC++;
                }
            }

            /* Handle destination operand */
            if (dst) {
                AREType are;
                unsigned short dst_encoded = encode_operand(dst, symbol_table, &are);
                add_to_memory_image(memory_image, dst_encoded, are, 0);
                IC++;

                if (dst_mode == DIRECT && dst[0] != '*') {
                    Symbol* symbol = find_symbol(symbol_table, dst);
                    if (symbol) {
                        add_to_memory_image(memory_image, (unsigned short)symbol->value, 
                                            symbol->is_external ? ARE_EXTERNAL : ARE_RELOCATABLE, 0);
                        if (symbol->is_external) {
                            add_external_reference(memory_image, symbol->name, IC);
                        }
                    } else {
                        report_error(line_number, "Symbol not found");
                        error_occurred = 1;
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
            error_occurred = 1;
        }
    }

    free(line);
    fclose(file);
    return error_occurred;
}

/* write_output_files: Generates .ob, .ent, and .ext output files */
void write_output_files(const char* filename, MemoryImage* memory_image, SymbolTable* symbol_table) {
    char *ob_filename, *ent_filename, *ext_filename;
    FILE *ob_file, *ent_file, *ext_file;
    int i, j;
    size_t filename_len;

    filename_len = strlen(filename);

    /* Allocate memory for filenames */
    ob_filename = (char*)malloc(filename_len + 4); /* +4 for ".ob\0" */
    ent_filename = (char*)malloc(filename_len + 5); /* +5 for ".ent\0" */
    ext_filename = (char*)malloc(filename_len + 5); /* +5 for ".ext\0" */

    if (!ob_filename || !ent_filename || !ext_filename) {
        report_error(0, "Memory allocation failed for filenames");
        free(ob_filename);
        free(ent_filename);
        free(ext_filename);
        return;
    }

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
        for (i = 0; i < symbol_table->count; i++) {
            if (symbol_table->symbols[i].is_external) {
                int found = 0;
                /* Check if the external symbol is used and write its addresses */
                for (j = 0; j < memory_image->external_count; j++) {
                    if (strcmp(symbol_table->symbols[i].name, memory_image->externals[j].symbol_name) == 0) {
                        fprintf(ext_file, "%s %04d\n", memory_image->externals[j].symbol_name, memory_image->externals[j].address);
                        found = 1;
                    }
                }
                /* If the external symbol is not used, write it with a special address */
                if (!found) {
                    fprintf(ext_file, "%s %04d\n", symbol_table->symbols[i].name, 0);  /* Use 0000 to indicate unused */
                }
            }
        }
        fclose(ext_file);
    } else {
        report_error(0, "Error opening .ext file for writing");
    }

    /* Free allocated memory */
    free(ob_filename);
    free(ent_filename);
    free(ext_filename);
}