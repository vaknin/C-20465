#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "preprocessing.h"
#include "first_pass.h"
#include "second_pass.h"

#define MAX_FILENAME 256

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename1> [filename2 ...]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        char input_filename[MAX_FILENAME];
        char am_filename[MAX_FILENAME];
        char ob_filename[MAX_FILENAME];
        char ent_filename[MAX_FILENAME];
        char ext_filename[MAX_FILENAME];
        
        // Construct input and output filenames
        snprintf(input_filename, sizeof(input_filename), "%s.as", argv[i]);
        snprintf(am_filename, sizeof(am_filename), "%s.am", argv[i]);
        snprintf(ob_filename, sizeof(ob_filename), "%s.ob", argv[i]);
        snprintf(ent_filename, sizeof(ent_filename), "%s.ent", argv[i]);
        snprintf(ext_filename, sizeof(ext_filename), "%s.ext", argv[i]);

        printf("Processing file: %s\n", input_filename);

        // Preprocessing phase
        printf("Starting preprocessing...\n");
        if (preprocess_file(input_filename) != 0) {
            printf("Error: Preprocessing failed for file %s\n", input_filename);
            continue;  // Move to the next file
        }
        printf("Preprocessing completed.\n");

        // First pass
        printf("Starting first pass...\n");
        SymbolTable symbol_table = create_symbol_table();
        if (first_pass(am_filename, &symbol_table) != 0) {
            printf("Error: First pass failed for file %s\n", am_filename);
            free_symbol_table(&symbol_table);
            continue;  // Move to the next file
        }
        printf("First pass completed.\n");

        // Second pass
        printf("Starting second pass...\n");
        MemoryImage memory_image = {0};
        if (second_pass(am_filename, &symbol_table, &memory_image) != 0) {
            printf("Error: Second pass failed for file %s\n", am_filename);
            free_symbol_table(&symbol_table);
            free(memory_image.code);
            free(memory_image.data);
            continue;  // Move to the next file
        }
        printf("Second pass completed.\n");

        // Generate output files
        printf("Generating output files...\n");
        write_output_files(argv[i], &memory_image, &symbol_table);
        printf("Output files generated.\n");

        // Clean up
        free_symbol_table(&symbol_table);
        free(memory_image.code);
        free(memory_image.data);

        printf("Processing completed for file: %s\n\n", input_filename);
    }

    return 0;
}