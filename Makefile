# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0` -lsqlite3 -lcrypto -lm

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
DATA_DIR = data

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# Resource files
RC_FILE = app.rc
RES_FILE = $(BUILD_DIR)/app.res

# Target executable
TARGET = $(BUILD_DIR)/assignment_tracker.exe

# Default target
all: $(BUILD_DIR) $(DATA_DIR) $(TARGET)

# Compile resource file
$(RES_FILE): $(RC_FILE) | $(BUILD_DIR)
	@echo "Compiling resource file..."
	windres $(RC_FILE) -O coff -o $(RES_FILE)

# Link object files to create executable
$(TARGET): $(OBJECTS) $(RES_FILE)
	@echo "Linking executable..."
	$(CC) $(OBJECTS) $(RES_FILE) -o $(TARGET) $(LDFLAGS)
	@echo "========================================="
	@echo "Build complete: $(TARGET)"
	@echo "Run with: make run"
	@echo "========================================="

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory if it doesn't exist
$(BUILD_DIR):
	@echo "Creating build directory..."
	@mkdir -p $(BUILD_DIR)

# Create data directory if it doesn't exist
$(DATA_DIR):
	@echo "Creating data directory..."
	@mkdir -p $(DATA_DIR)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)/*.o $(TARGET)
	@echo "Clean complete"

# Run the application
run: $(TARGET)
	@echo "Starting Assignment Tracker..."
	@$(TARGET)

# Rebuild everything
rebuild: clean all

# Show help
help:
	@echo "Assignment Tracker - Makefile targets:"
	@echo "  make all      - Build the project (default)"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make run      - Build and run the application"
	@echo "  make rebuild  - Clean and rebuild everything"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Build configuration:"
	@echo "  Compiler: $(CC)"
	@echo "  Source dir: $(SRC_DIR)"
	@echo "  Build dir: $(BUILD_DIR)"
	@echo "  Target: $(TARGET)"

.PHONY: all clean run rebuild help
