#include "helper_functions.h"
#include "assembler.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static int error_count = 0;

void report_error(int line_number, const char* message) {
    fprintf(stdout, "Error at line %d: %s\n", line_number, message);
    error_count++;
}

int get_error_count() {
    return error_count;
}

void reset_error_count() {
    error_count = 0;
}

bool is_valid_label(const char* label) {
    if (!label || strlen(label) > 31 || !isalpha(label[0])) {
        return false;
    }
    
    for (int i = 1; label[i]; i++) {
        if (!isalnum(label[i])) {
            return false;
        }
    }
    
    // Check if the label is a reserved word (this list should be expanded)
    const char* reserved_words[] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
    int num_reserved = sizeof(reserved_words) / sizeof(reserved_words[0]);
    
    for (int i = 0; i < num_reserved; i++) {
        if (strcmp(label, reserved_words[i]) == 0) {
            return false;
        }
    }
    
    return true;
}

char* trim(char* str) {
    if (!str) return NULL;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0)  // All spaces?
        return str;
    
    // Trim trailing space
    char* end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator character
    end[1] = '\0';
    
    return str;
}

char* str_tolower(char* str) {
    for(char *p = str; *p; p++) {
        *p = tolower((unsigned char)*p);
    }
    return str;
}

char* skip_whitespace(char* str) {
    while (*str && isspace((unsigned char)*str)) str++;
    return str;
}

char* get_next_token(char** str) {
    *str = skip_whitespace(*str);
    if (**str == '\0') return NULL;

    char* start = *str;
    if (**str == '"') {
        (*str)++;
        while (**str && **str != '"') (*str)++;
        if (**str == '"') (*str)++;
    } else {
        while (**str && !isspace((unsigned char)**str) && **str != ',') (*str)++;
    }
    
    if (*str != start) {
        char* token = strndup(start, *str - start);
        *str = skip_whitespace(*str);
        if (**str == ',') (*str)++;
        return token;
    }
    return NULL;
}

bool is_label(const char* token) {
    return token[strlen(token) - 1] == ':';
}

bool is_data_directive(const char* token) {
    return strcmp(token, ".data") == 0 || strcmp(token, ".string") == 0;
}

bool is_extern_entry_directive(const char* token) {
    return strcmp(token, ".extern") == 0 || strcmp(token, ".entry") == 0;
}

int count_data_words(const char* line) {
    int count = 0;
    const char* p = line;
    while (*p) {
        if (*p == '"') {  // String
            p++;
            while (*p && *p != '"') {
                count++;
                p++;
            }
            count++; // For null terminator
        } else if (isdigit(*p) || *p == '-' || *p == '+') {
            count++;
            while (isdigit(*p)) p++;
        }
        p++;
    }
    return count;
}

int get_opcode(const char* instruction) {
    char opcode[MAX_LABEL_LENGTH];
    sscanf(instruction, "%s", opcode);
    
    for (int i = 0; i < NUM_OPCODES; i++) {
        if (strcmp(opcode, OPCODE_STRINGS[i]) == 0) {
            return i;
        }
    }
    return -1;
}

int get_register_number(const char* reg) {
    if (reg[0] == 'r' && isdigit(reg[1])) {
        int num = atoi(reg + 1);
        if (num >= 0 && num < NUM_REGISTERS) {
            return num;
        }
    }
    return -1;
}

int encode_addressing_mode(const char* operand) {
    if (operand[0] == '#') return IMMEDIATE;
    if (operand[0] == 'r' && get_register_number(operand) != -1) return REGISTER;
    if (operand[0] == '*' && operand[1] == 'r' && get_register_number(operand + 1) != -1) return DIRECT;  // Register indirect
    if (strchr(operand, '[') && strchr(operand, ']')) return INDEX;
    return DIRECT;
}

short twos_complement(int value) {
    if (value < 0) {
        return (short)((1 << (WORD_SIZE - 1)) + value);
    }
    return (short)value;
}

char* strip_comments(char* line) {
    bool in_string = false;
    char* p;
    for (p = line; *p; p++) {
        if (*p == '"') {
            in_string = !in_string;
        } else if (*p == ';' && !in_string) {
            *p = '\0';
            break;
        }
    }
    
    // Trim trailing whitespace, but preserve newline
    while (p > line && isspace((unsigned char)*(p-1)) && *(p-1) != '\n') {
        p--;
        *p = '\0';
    }

    // Trim leading whitespace
    char* start = line;
    while (*start && isspace((unsigned char)*start) && *start != '\n') start++;

    if (start != line) {
        memmove(line, start, strlen(start) + 1);
    }

    return line;
}

char* get_first_token(const char* str) {
    const char* start = str;
    while (isspace((unsigned char)*start)) start++;
    if (*start == '\0') return NULL;

    const char* end = start;
    while (*end && !isspace((unsigned char)*end)) end++;

    return strndup(start, end - start);
}

char* get_indentation(const char* str) {
    const char* end = str;
    while (*end && isspace((unsigned char)*end)) end++;
    return strndup(str, end - str);
}