#include "symbol_table.h"
#include "first_pass.h"
#include "second_pass.h"
#include "macro_expander.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 1;  // No input files provided
    }

    // Process each file provided as an argument
    for (int i = 1; i < argc; i++) {
        SymbolTable table;
        initSymbolTable(&table);

        // Perform the first pass which includes macro expansion and symbol table population
        if (firstPass(argv[i], &table) != 0) {
            return 1;  // Exit if there's an error in the first pass
        }

        // Perform the second pass to generate the .ob, .ext, and .ent files
        if (secondPass(argv[i], &table) != 0) {
            return 1;  // Exit if there's an error in the second pass
        }
    }

    return 0;
}