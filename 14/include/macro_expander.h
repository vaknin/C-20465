#ifndef MACRO_EXPANDER_H
#define MACRO_EXPANDER_H

#include <stdio.h>

// Function to expand macros in a source file and generate the .am file
int expandMacros(const char *filenamePrefix);

#endif // MACRO_EXPANDER_H