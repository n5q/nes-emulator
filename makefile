# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Iinclude -Werror -Wpedantic -lSDL2

# Directories
SRC_DIR := src
BIN_DIR := bin

# Target
TARGET := $(BIN_DIR)/nes

# Recursively find all .cc sources (includes src/mappers/*.cc)
SRC := $(shell find $(SRC_DIR) -type f -name '*.cc')

# Map src/.../*.cc -> bin/.../*.o
OBJ := $(patsubst $(SRC_DIR)/%.cc,$(BIN_DIR)/%.o,$(SRC))

# Default rule
all: $(TARGET)

# Link object files to create binary
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files (and create subdirs in bin/)
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BIN_DIR) $(TARGET)

.PHONY: all clean
