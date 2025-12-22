# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Iinclude

# Directories
SRC_DIR := src
INC_DIR := include
BIN_DIR := bin

# Target
TARGET := $(BIN_DIR)/nes

# Source and object files
SRC := $(wildcard $(SRC_DIR)/*.cc)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)

# Default rule
all: $(TARGET)

# Ensure bin directory exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Link object files to create binary
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BIN_DIR)/*.o $(TARGET)

# Phony targets
.PHONY: all clean
