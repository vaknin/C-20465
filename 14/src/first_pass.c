#include <stdio.h>
#include <string.h>
#include "first_pass.h"
#include "macro_expander.h"
#include "symbol_table.h"

#define MAX_LINE_LENGTH 256

// Function to perform the first pass of the assembler
int firstPass(const char *filenamePrefix, SymbolTable *symbolTable) {
    // Expand macros from .as to .am
    if (expandMacros(filenamePrefix) != 0) {
        printf("Error: Failed to expand macros in %s.as\n", filenamePrefix);
        return -1;
    }

    // Generate the .am filename
    char amFileName[256];
    strcpy(amFileName, filenamePrefix);
    strcat(amFileName, ".am");

    // Open the .am file for processing
    FILE *amFile = fopen(amFileName, "r");
    if (amFile == NULL) {
        printf("Error: Could not open file %s\n", amFileName);
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    int IC = 100;  // Instruction Counter starts at address 100
    int DC = 0;    // Data Counter starts at 0
    int lineNumber = 0;

    while (fgets(line, sizeof(line), amFile)) {
        lineNumber++;
        line[strcspn(line, "\n")] = 0; // Strip newline character

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == ';') {
            continue;
        }

        char *token = strtok(line, " \t");
        char label[31] = "";

        // Check if the first field is a label
        if (strchr(token, ':')) {
            strncpy(label, token, strlen(token) - 1);  // Copy label without ':'
            token = strtok(NULL, " \t");
        }

        // Handle .data or .string directives
        if (token && (strcmp(token, ".data") == 0 || strcmp(token, ".string") == 0)) {
            if (label[0] != '\0') {
                if (addSymbol(symbolTable, label, DC + IC) != 0) {
                    printf("Error on line %d: Duplicate label '%s'\n", lineNumber, label);
                    fclose(amFile);
                    return -1;
                }
            }
            // Update DC for .data and .string
            if (strcmp(token, ".data") == 0) {
                while ((token = strtok(NULL, ",")) != NULL) {
                    DC += 1;
                }
            } else if (strcmp(token, ".string") == 0) {
                token = strtok(NULL, "\"");
                DC += strlen(token) + 1;  // Add 1 for the null terminator
            }
        }
        // Handle .extern directive
        else if (token && strcmp(token, ".extern") == 0) {
            while ((token = strtok(NULL, " \t,")) != NULL) {
                if (addSymbol(symbolTable, token, 0) != 0) {
                    printf("Error on line %d: Duplicate extern label '%s'\n", lineNumber, token);
                    fclose(amFile);
                    return -1;
                }
            }
        }
        // Handle .entry directive (entries are resolved in the second pass)
        else if (token && strcmp(token, ".entry") == 0) {
            // Entries are noted but not resolved here
        }
        // Handle instructions
        else if (token) {
            if (label[0] != '\0') {
                if (addSymbol(symbolTable, label, IC) != 0) {
                    printf("Error on line %d: Duplicate label '%s'\n", lineNumber, label);
                    fclose(amFile);
                    return -1;
                }
            }
            IC += 1;  // Assume 1 word per instruction (adjust if instructions have varying lengths)
        }
    }

    // Final adjustment: update addresses of data symbols
    for (int i = 0; i < symbolTable->count; i++) {
        if (symbolTable->symbols[i].address >= 100) {  // Skip code labels
            continue;
        }
        symbolTable->symbols[i].address += IC;  // Adjust data symbols' addresses
    }

    fclose(amFile);
    return 0;
}