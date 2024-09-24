/***************************************************************
 * File: helper_functions.c
 * Description: Implements utility functions for the assembler project.
 ***************************************************************/

#include "helper_functions.h"
#include "assembler.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static int error_count = 0; /* Keeps track of the number of errors encountered */

/* report_error: Reports an error with the given line number and message. */
void report_error(int line_number, const char* message)
{
    fprintf(stdout, "Error at line %d: %s\n", line_number, message);
    error_count++; /* Increment the error count */
}

/* get_error_count: Returns the current count of reported errors. */
int get_error_count(void)
{
    return error_count;
}

/* reset_error_count: Resets the error count to zero. */
void reset_error_count(void)
{
    error_count = 0;
}

/* is_valid_label: Checks if the given string is a valid label. */
bool is_valid_label(const char* label)
{
    int i;
    /* List of reserved words that cannot be used as labels */
    const char* reserved_words[] = {
        "mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec",
        "jmp", "bne", "red", "prn", "jsr", "rts", "stop",
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
    };
    int num_reserved = sizeof(reserved_words) / sizeof(reserved_words[0]);

    /* Check if label is NULL, too long, or doesn't start with a letter */
    if (!label || strlen(label) > 31 || !isalpha((unsigned char)label[0])) {
        return false;
    }
    
    /* Check if all characters after the first are alphanumeric */
    for (i = 1; label[i]; i++) {
        if (!isalnum((unsigned char)label[i])) {
            return false;
        }
    }
    
    /* Check if the label is a reserved word */
    for (i = 0; i < num_reserved; i++) {
        if (strcmp(label, reserved_words[i]) == 0) {
            return false;
        }
    }
    
    return true; /* Label is valid if it passed all checks */
}

/* trim: Removes leading and trailing whitespace from a string. */
char* trim(char* str)
{
    char* end;

    if (!str) return NULL;
    
    /* Trim leading space */
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return str; /* All spaces? */
    
    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    /* Write new null terminator character */
    end[1] = '\0';
    
    return str;
}

/* str_tolower: Converts all characters in a string to lowercase. */
char* str_tolower(char* str)
{
    char *p;
    for(p = str; *p; p++) {
        *p = tolower((unsigned char)*p);
    }
    return str;
}

/* skip_whitespace: Advances a string pointer past any leading whitespace. */
char* skip_whitespace(char* str)
{
    while (*str && isspace((unsigned char)*str)) str++;
    return str;
}

/* get_next_token: Extracts the next token from a string. */
char* get_next_token(char** str)
{
    char* start;
    char* token;

    *str = skip_whitespace(*str);
    if (**str == '\0') return NULL;

    start = *str;
    if (**str == '"') {
        /* Handle quoted strings */
        (*str)++;
        while (**str && **str != '"') (*str)++;
        if (**str == '"') (*str)++;
    } else {
        /* Handle regular tokens */
        while (**str && !isspace((unsigned char)**str) && **str != ',') (*str)++;
    }
    
    if (*str != start) {
        /* Allocate and copy the token */
        token = malloc((*str - start + 1) * sizeof(char));
        if (token == NULL) {
            fprintf(stderr, "Memory allocation failed in get_next_token\n");
            exit(1);
        }
        strncpy(token, start, *str - start);
        token[*str - start] = '\0';
        *str = skip_whitespace(*str);
        if (**str == ',') (*str)++; /* Skip comma if present */
        return token;
    }
    return NULL;
}

/* is_label: Checks if the given token is a label. */
bool is_label(const char* token)
{
    return token[strlen(token) - 1] == ':';
}

/* is_data_directive: Checks if the token is a data directive (.data or .string). */
bool is_data_directive(const char* token)
{
    return strcmp(token, ".data") == 0 || strcmp(token, ".string") == 0;
}

/* is_extern_entry_directive: Checks if the token is an extern or entry directive. */
bool is_extern_entry_directive(const char* token)
{
    return strcmp(token, ".extern") == 0 || strcmp(token, ".entry") == 0;
}

/* count_data_words: Counts the number of data words in a .data or .string directive. */
int count_data_words(const char* line)
{
    int count = 0;
    const char* p = line;

    while (*p) {
        if (*p == '"') {  /* String */
            p++;
            while (*p && *p != '"') {
                count++;
                p++;
            }
            count++; /* For null terminator */
        } else if (isdigit((unsigned char)*p) || *p == '-' || *p == '+') {
            count++; /* Count number */
            while (isdigit((unsigned char)*p)) p++;
        }
        p++;
    }
    return count;
}

/* get_opcode: Retrieves the opcode for a given instruction. */
int get_opcode(const char* instruction)
{
    char opcode[MAX_LABEL_LENGTH];
    int i;

    sscanf(instruction, "%s", opcode);
    
    /* Search for the opcode in the OPCODE_STRINGS array */
    for (i = 0; i < NUM_OPCODES; i++) {
        if (strcmp(opcode, OPCODE_STRINGS[i]) == 0) {
            return i;
        }
    }
    return -1; /* Opcode not found */
}

/* get_register_number: Retrieves the register number from a register operand. */
int get_register_number(const char* reg)
{
    int num;

    if (reg[0] == 'r' && isdigit((unsigned char)reg[1])) {
        num = atoi(reg + 1);
        if (num >= 0 && num < NUM_REGISTERS) {
            return num;
        }
    }
    return -1; /* Invalid register */
}

/* encode_addressing_mode: Determines the addressing mode of an operand. */
int encode_addressing_mode(const char* operand)
{
    if (operand[0] == '#') return IMMEDIATE;
    if (operand[0] == 'r' && get_register_number(operand) != -1) return REGISTER;
    if (operand[0] == '*' && operand[1] == 'r' && get_register_number(operand + 1) != -1) return DIRECT;  /* Register indirect */
    if (strchr(operand, '[') && strchr(operand, ']')) return INDEX;
    return DIRECT;
}

/* twos_complement: Converts a value to its two's complement representation. */
short twos_complement(int value)
{
    if (value < 0) {
        return (short)((1 << (WORD_SIZE - 1)) + value);
    }
    return (short)value;
}

/* strip_comments: Removes comments from a line of assembly code. */
char* strip_comments(char* line)
{
    bool in_string = false;
    char* p;
    char* start;

    for (p = line; *p; p++) {
        if (*p == '"') {
            in_string = !in_string; /* Toggle in_string flag */
        } else if (*p == ';' && !in_string) {
            *p = '\0'; /* Terminate string at comment start */
            break;
        }
    }
    
    /* Trim trailing whitespace, but preserve newline */
    while (p > line && isspace((unsigned char)*(p-1)) && *(p-1) != '\n') {
        p--;
        *p = '\0';
    }

    /* Trim leading whitespace */
    start = line;
    while (*start && isspace((unsigned char)*start) && *start != '\n') start++;

    if (start != line) {
        memmove(line, start, strlen(start) + 1);
    }

    return line;
}

/* get_first_token: Extracts the first token from a string. */
char* get_first_token(const char* str)
{
    const char* start;
    const char* end;
    char* token;

    /* Find start of first non-whitespace character */
    start = str;
    while (isspace((unsigned char)*start)) start++;
    if (*start == '\0') return NULL;

    /* Find end of token */
    end = start;
    while (*end && !isspace((unsigned char)*end)) end++;

    /* Allocate and copy the token */
    token = malloc((end - start + 1) * sizeof(char));
    if (token == NULL) {
        fprintf(stderr, "Memory allocation failed in get_first_token\n");
        exit(1);
    }
    strncpy(token, start, end - start);
    token[end - start] = '\0';
    return token;
}

/* get_indentation: Extracts the leading whitespace from a string. */
char* get_indentation(const char* str)
{
    const char* end;
    char* indentation;

    /* Find end of leading whitespace */
    end = str;
    while (*end && isspace((unsigned char)*end)) end++;

    /* Allocate and copy the indentation */
    indentation = malloc((end - str + 1) * sizeof(char));
    if (indentation == NULL) {
        fprintf(stderr, "Memory allocation failed in get_indentation\n");
        exit(1);
    }
    strncpy(indentation, str, end - str);
    indentation[end - str] = '\0';
    return indentation;
}