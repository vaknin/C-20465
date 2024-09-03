CC = gcc
CFLAGS = -Iinclude

SRC_DIR = src
INCLUDE_DIR = include

# List of source files
SRCS = $(wildcard $(SRC_DIR)/*)

# Output executable
TARGET = assembler

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)