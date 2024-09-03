#include <stdio.h>
#include <string.h>
#include "second_pass.h"
#include "symbol_table.h"

// Function to perform the second pass of the assembler
int secondPass(const char *filename, SymbolTable *symbolTable) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return -1;
    }

    char line[256];
    int address = 100;  // Starting address for instructions
    int lineNumber = 0;

    while (fgets(line, sizeof(line), file)) {
        lineNumber++;
        line[strcspn(line, "\n")] = 0; // Strip newline character

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == ';') {
            continue;
        }

        char *token = strtok(line, " \t");

        // Skip label (if present)
        if (strchr(token, ':')) {
            token = strtok(NULL, " \t");
        }

        // Handle instructions (assume each instruction takes 1 word for simplicity)
        if (token) {
            // Here you would generate the machine code for each instruction using the symbol table
            // Example (very simplified):
            printf("Processing instruction: %s at address %d\n", token, address);

            // For the purpose of this example, we'll simply increment the address
            address += 1;
        }
    }

    fclose(file);
    return 0;
}