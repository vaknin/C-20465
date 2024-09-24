/***************************************************************
 * File: helper_functions.h
 * Description: Declares utility functions for the assembler project.
 ***************************************************************/

#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

/* Define boolean type for ANSI C (C89/C90) compliance */
typedef int bool;
#define true 1
#define false 0

/* report_error: Reports an error with the given line number and message. */
void report_error(int line_number, const char* message);

/* get_error_count: Returns the current count of reported errors. */
int get_error_count(void);

/* reset_error_count: Resets the error count to zero. */
void reset_error_count(void);

/* is_valid_label: Checks if the given string is a valid label. */
bool is_valid_label(const char* label);

/* trim: Removes leading and trailing whitespace from a string. */
char* trim(char* str);

/* str_tolower: Converts all characters in a string to lowercase. */
char* str_tolower(char* str);

/* skip_whitespace: Advances a string pointer past any leading whitespace. */
char* skip_whitespace(char* str);

/* get_next_token: Extracts the next token from a string. */
char* get_next_token(char** str);

/* is_label: Checks if the given token is a label. */
bool is_label(const char* token);

/* is_data_directive: Checks if the token is a data directive (.data or .string). */
bool is_data_directive(const char* token);

/* is_extern_entry_directive: Checks if the token is an extern or entry directive. */
bool is_extern_entry_directive(const char* token);

/* count_data_words: Counts the number of data words in a .data or .string directive. */
int count_data_words(const char* line);

/* get_opcode: Retrieves the opcode for a given instruction. */
int get_opcode(const char* instruction);

/* get_register_number: Retrieves the register number from a register operand. */
int get_register_number(const char* reg);

/* encode_addressing_mode: Determines the addressing mode of an operand. */
int encode_addressing_mode(const char* operand);

/* twos_complement: Converts a value to its two's complement representation. */
short twos_complement(int value);

/* strip_comments: Removes comments from a line of assembly code. */
char* strip_comments(char* line);

/* get_first_token: Extracts the first token from a string. */
char* get_first_token(const char* str);

/* get_indentation: Extracts the leading whitespace from a string. */
char* get_indentation(const char* str);

#endif /* HELPER_FUNCTIONS_H */