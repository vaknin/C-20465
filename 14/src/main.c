#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "preprocessing.h"
#include "first_pass.h"
#include "second_pass.h"
#include "helper_functions.h"

char* create_filename(const char* base, const char* extension) {
    size_t len = strlen(base) + (extension ? strlen(extension) : 0) + 1;
    char* filename = malloc(len);
    if (!filename) {
        perror("Memory allocation failed");
        exit(1);
    }
    strcpy(filename, base);
    if (extension) {
        strcat(filename, extension);
    }
    return filename;
}

int process_file(const char* base_filename) {
    char* input_filename = create_filename(base_filename, ".as");
    FILE* test_file = fopen(input_filename, "r");
    if (!test_file) {
        free(input_filename);
        input_filename = strdup(base_filename);
        test_file = fopen(input_filename, "r");
        if (!test_file) {
            printf("Error: Cannot open input file %s or %s.as\n", base_filename, base_filename);
            free(input_filename);
            return 1;
        }
    }
    fclose(test_file);

    char* am_filename = create_filename(base_filename, ".am");

    printf("Processing file: %s\n", input_filename);

    reset_error_count();  // Reset error count for this file

    printf("Starting preprocessing...\n");
    if (preprocess_file(input_filename) != 0) {
        printf("Error: Preprocessing failed for file %s\n", input_filename);
        free(input_filename);
        free(am_filename);
        return 1;
    }
    printf("Preprocessing completed.\n");

    printf("Starting first pass...\n");
    SymbolTable symbol_table = create_symbol_table();
    first_pass(am_filename, &symbol_table);

    if (get_error_count() == 0) {
        printf("First pass completed successfully.\n");

        printf("Starting second pass...\n");
        MemoryImage memory_image;
        init_memory_image(&memory_image);
        second_pass(am_filename, &symbol_table, &memory_image);

        if (get_error_count() == 0) {
            printf("Second pass completed successfully.\n");

            printf("Generating output files...\n");
            write_output_files(base_filename, &memory_image, &symbol_table);
            printf("Output files generated.\n");
        }

        free_memory_image(&memory_image);
    }

    free_symbol_table(&symbol_table);

    if (get_error_count() > 0) {
        printf("Errors were found during processing. No output files generated for %s.\n", base_filename);
    }

    printf("Processing completed for file: %s\n\n", base_filename);

    free(input_filename);
    free(am_filename);

    return get_error_count() > 0 ? 1 : 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename1> [filename2 ...]\n", argv[0]);
        return 1;
    }

    int total_files = 0;
    int files_with_errors = 0;

    for (int i = 1; i < argc; i++) {
        printf("Processing file %d of %d: %s\n", i, argc - 1, argv[i]);
        int result = process_file(argv[i]);
        total_files++;
        if (result != 0) {
            files_with_errors++;
        }
        printf("\n");
    }

    printf("Assembly process completed.\n");
    printf("Total files processed: %d\n", total_files);
    printf("Files assembled successfully: %d\n", total_files - files_with_errors);
    printf("Files with errors: %d\n", files_with_errors);

    return files_with_errors > 0 ? 1 : 0;
}