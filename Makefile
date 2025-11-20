# ================================
#  Banking Fraud Detection Project
#  Makefile
# ================================

# Compiler
CC      := clang
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude
LDFLAGS := 

# Directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Target executable
TARGET  := $(BIN_DIR)/fraud_detect

# Find all .c files
SRCS := $(wildcard $(SRC_DIR)/*.c)

# Convert src/*.c → obj/*.o
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Dependency files
DEPS := $(OBJS:.o=.d)

# ================================
#  Default build
# ================================
all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "[OK] Build complete → $(TARGET)"

# ================================
#  Compile object files + deps
# ================================
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# ================================
#  Create directories if missing
# ================================
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ================================
#  Debug version (no optimizations)
# ================================
debug: CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g -O0 -Iinclude
debug: clean $(TARGET)
	@echo "[DEBUG] Built with debug flags."

# ================================
#  Run the program
# ================================
run: all
	./$(TARGET)

# ================================
#  Cleanup
# ================================
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "[CLEAN] All build artifacts removed."

# ================================
#  Include auto-generated deps
# ================================
-include $(DEPS)

.PHONY: all clean debug run
