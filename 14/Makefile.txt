# Define the compiler
CC=gcc

# Define any compile-time flags
CFLAGS=-Wall -ansi -pedantic

# Define the target executable name
TARGET=partialSums

# Define source files
SRC=partialSums.c

# Default target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)