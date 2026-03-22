# généré par ChatGPT5 (amélioré par le Grand et Valeureux Maître Beat)
#
# project/
# ├── bin/               # Directory for the compiled binary and object files
# │   ├── obj/           # Subdirectory for object files
# │   │   ├── main.o     # Object file for main.cpp
# │   │   └── other.o    # Object files for each source file in src/
# │   └── main           # The executable
# ├── headers/           # Directory for header files (.h)
# │   └── some.h
# ├── src/               # Directory for source files (.cpp)
# │   └── some.cpp
# ├── main.cpp
# └── Makefile           # The Makefile itself
#

# Compiler
CC = g++

# Flags for compilation and linking
CFLAGS = -c -I$(HEADER_DIR) -fopenmp -O3 -g -march=native -Wno-unused-result
LDFLAGS = -lm -lGL -lGLU -lglut -lpthread -fopenmp 

# Directories
SRC_DIR = src
HEADER_DIR = headers
BIN_DIR = bin
OBJ_DIR = $(BIN_DIR)/obj

# Detect all source files recursively (finds both main.cpp and src/*.cpp)
SRC = $(wildcard *.cpp) $(wildcard $(SRC_DIR)/*.cpp)

# Define the object files (place .o files in OBJ_DIR)
OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRC)))

# Name of the executable
TARGET = $(BIN_DIR)/main

# Default target
all: $(OBJ_DIR) $(TARGET)

# Create the executable from all object files
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# Rule to compile each source file into OBJ_DIR
$(OBJ_DIR)/%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

# Rule to compile each source file from src/ into OBJ_DIR
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

# Clean target to remove compiled objects and the binary
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET) loss.txt loss.png



# Create the obj directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Run the program and save loss data
run: $(TARGET)
	@$(TARGET)

debug: $(TARGET)
	@gdb -q $(TARGET)

# Phony targets
.PHONY: all clean run
