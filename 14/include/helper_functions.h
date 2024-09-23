#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

/* Define boolean type for ANSI C (C89/C90) compliance */
typedef int bool;
#define true 1
#define false 0
void report_error(int line_number, const char* message);
int get_error_count();
void reset_error_count();
bool is_valid_label(const char* label);
char* trim(char* str);
char* str_tolower(char* str);
char* skip_whitespace(char* str);
char* get_next_token(char** str);
bool is_label(const char* token);
bool is_data_directive(const char* token);
bool is_extern_entry_directive(const char* token);
int count_data_words(const char* line);
int get_opcode(const char* instruction);
int get_register_number(const char* reg);
int encode_addressing_mode(const char* operand);
short twos_complement(int value);
char* strip_comments(char* line);
char* get_first_token(const char* str);
char* get_indentation(const char* str);

#endif /* HELPER_FUNCTIONS_H */