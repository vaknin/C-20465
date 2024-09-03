#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "macro_expander.h"

#define MAX_LINE_LENGTH 256
#define MAX_MACRO_NAME 31
#define MAX_MACRO_LINES 100
#define MAX_MACROS 20

typedef struct {
    char name[MAX_MACRO_NAME];
    char lines[MAX_MACRO_LINES][MAX_LINE_LENGTH];
    int lineCount;
} Macro;

static Macro macros[MAX_MACROS];
static int macroCount = 0;

// Helper functions to handle macros
static int isMacroDefinitionStart(const char *line) {
    return strncmp(line, "macro", 5) == 0;
}

static int isMacroDefinitionEnd(const char *line) {
    return strcmp(line, "endmacro") == 0;
}

static Macro* findMacro(const char *name) {
    for (int i = 0; i < macroCount; i++) {
        if (strcmp(macros[i].name, name) == 0) {
            return &macros[i];
        }
    }
    return NULL;
}

// Expand macros in the source file and write the result to the .am file
int expandMacros(const char *filenamePrefix) {
    char sourceFileName[256];
    char outputFileName[256];

    // Generate filenames: source (.as) and output (.am)
    strcpy(sourceFileName, filenamePrefix);
    strcat(sourceFileName, ".as");
    strcpy(outputFileName, filenamePrefix);
    strcat(outputFileName, ".am");

    FILE *sourceFile = fopen(sourceFileName, "r");
    FILE *amFile = fopen(outputFileName, "w");
    if (sourceFile == NULL || amFile == NULL) {
        printf("Error: Could not open file %s or %s\n", sourceFileName, outputFileName);
        if (sourceFile) fclose(sourceFile);
        if (amFile) fclose(amFile);
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    int inMacroDefinition = 0;
    Macro *currentMacro = NULL;

    while (fgets(line, sizeof(line), sourceFile)) {
        line[strcspn(line, "\n")] = 0; // Strip newline character

        if (inMacroDefinition) {
            if (isMacroDefinitionEnd(line)) {
                inMacroDefinition = 0;
                currentMacro = NULL;
            } else if (currentMacro && currentMacro->lineCount < MAX_MACRO_LINES) {
                strcpy(currentMacro->lines[currentMacro->lineCount++], line);
            }
            continue;
        }

        Macro *macro = findMacro(line);
        if (macro) {
            for (int i = 0; i < macro->lineCount; i++) {
                fprintf(amFile, "%s\n", macro->lines[i]);
            }
            continue;
        }

        if (isMacroDefinitionStart(line)) {
            if (macroCount < MAX_MACROS) {
                currentMacro = &macros[macroCount++];
                sscanf(line, "macro %30s", currentMacro->name);
                currentMacro->lineCount = 0;
                inMacroDefinition = 1;
            } else {
                printf("Error: Too many macros defined\n");
                fclose(sourceFile);
                fclose(amFile);
                return -1;
            }
            continue;
        }

        fprintf(amFile, "%s\n", line);
    }

    fclose(sourceFile);
    fclose(amFile);
    return 0;
}