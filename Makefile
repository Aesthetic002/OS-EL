# OS-EL Deadlock Detection & Recovery Module
# Makefile for Windows (MinGW) and Linux/macOS

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I.
LDFLAGS =

# Debug/Release mode
ifdef DEBUG
    CFLAGS += -g -DDEBUG -O0
else
    CFLAGS += -O2 -DNDEBUG
endif

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = tests

# Source files
SOURCES = $(SRC_DIR)/rag.c \
          $(SRC_DIR)/cycle_detector.c \
          $(SRC_DIR)/recovery.c \
          $(SRC_DIR)/simulator.c \
          $(SRC_DIR)/api.c

# Object files
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

# Main executable
MAIN_SRC = main.c
MAIN_OBJ = $(OBJ_DIR)/main.o
TARGET = $(BIN_DIR)/deadlock

# Test executable
TEST_SRC = $(TEST_DIR)/test_deadlock.c
TEST_OBJ = $(OBJ_DIR)/test_deadlock.o
TEST_TARGET = $(BIN_DIR)/test_deadlock

# Detect OS
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    TEST_TARGET := $(TEST_TARGET).exe
    RM = del /Q /F
    RMDIR = rmdir /S /Q
    SEP = \\
else
    RM = rm -f
    RMDIR = rm -rf
    SEP = /
endif

# Default target
.PHONY: all
all: dirs $(TARGET)

# Create directories
.PHONY: dirs
ifeq ($(OS),Windows_NT)
dirs:
	@cmd /c "if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)"
	@cmd /c "if not exist $(BIN_DIR) mkdir $(BIN_DIR)"
else
dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)
endif

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

# Compile main
$(MAIN_OBJ): $(MAIN_SRC) $(INC_DIR)/*.h
	$(CC) $(CFLAGS) -c $< -o $@

# Link main executable
$(TARGET): $(OBJECTS) $(MAIN_OBJ)
	$(CC) $(OBJECTS) $(MAIN_OBJ) $(LDFLAGS) -o $@
	@echo Build complete: $(TARGET)

# Test target
.PHONY: test
test: dirs $(TEST_TARGET)
	@echo Running tests...
	@$(TEST_TARGET)

$(TEST_OBJ): $(TEST_SRC) $(INC_DIR)/*.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_TARGET): $(OBJECTS) $(TEST_OBJ)
	$(CC) $(OBJECTS) $(TEST_OBJ) $(LDFLAGS) -o $@

# Debug build
.PHONY: debug
debug: DEBUG=1
debug: all

# Clean build artifacts
.PHONY: clean
clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(OBJ_DIR)" $(RMDIR) "$(OBJ_DIR)"
	@if exist "$(BIN_DIR)" $(RMDIR) "$(BIN_DIR)"
else
	@$(RMDIR) $(OBJ_DIR) $(BIN_DIR)
endif
	@echo Clean complete

# Rebuild from scratch
.PHONY: rebuild
rebuild: clean all

# Run the main program
.PHONY: run
run: all
	@$(TARGET)

# Run in API mode (for Python GUI)
.PHONY: api
api: all
	@$(TARGET) --api

# Install (copy to system location - Linux/macOS only)
.PHONY: install
install: all
ifndef $(OS),Windows_NT
	install -m 755 $(TARGET) /usr/local/bin/os-el-deadlock
	@echo Installed to /usr/local/bin/os-el-deadlock
endif

# Demo targets
.PHONY: demo
demo: all
	@echo "=== Running Simple Deadlock Demo ==="
	@$(TARGET)

# Help
.PHONY: help
help:
	@echo ""
	@echo "OS-EL Deadlock Detection & Recovery Module - Build System"
	@echo "==========================================================="
	@echo ""
	@echo "Available targets:"
	@echo "  make          - Build the main executable"
	@echo "  make debug    - Build with debug symbols"
	@echo "  make test     - Build and run unit tests"
	@echo "  make run      - Build and run the CLI demo"
	@echo "  make api      - Build and run in API server mode"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make rebuild  - Clean and rebuild"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Options:"
	@echo "  DEBUG=1       - Enable debug build (e.g., make DEBUG=1)"
	@echo ""

# Dependencies
$(OBJ_DIR)/rag.o: $(SRC_DIR)/rag.c $(INC_DIR)/rag.h
$(OBJ_DIR)/cycle_detector.o: $(SRC_DIR)/cycle_detector.c $(INC_DIR)/cycle_detector.h $(INC_DIR)/rag.h
$(OBJ_DIR)/recovery.o: $(SRC_DIR)/recovery.c $(INC_DIR)/recovery.h $(INC_DIR)/rag.h $(INC_DIR)/cycle_detector.h
$(OBJ_DIR)/simulator.o: $(SRC_DIR)/simulator.c $(INC_DIR)/simulator.h $(INC_DIR)/rag.h $(INC_DIR)/cycle_detector.h $(INC_DIR)/recovery.h
$(OBJ_DIR)/api.o: $(SRC_DIR)/api.c $(INC_DIR)/api.h $(INC_DIR)/rag.h $(INC_DIR)/cycle_detector.h $(INC_DIR)/recovery.h $(INC_DIR)/simulator.h
