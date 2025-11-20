.PHONY: all build clean run help

CC := clang
CFLAGS := -Wall -Wextra -O2 -std=c99 -I./include
LDFLAGS := -lm -lcjson

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -I/opt/homebrew/include
    LDFLAGS += -L/opt/homebrew/lib
endif

SRC := src/main.c src/graph.c src/algorithms.c src/utils.c
OBJ := $(SRC:src/%.c=build/%.o)
TARGET := bin/fraud_detector

all: build

build: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p bin
	@echo "[LINK] $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✓ Build successful"

build/%.o: src/%.c
	@mkdir -p build
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	@./$(TARGET)

clean:
	@rm -rf build bin
	@echo "✓ Clean complete"

help:
	@echo "Fraud Detection Benchmark"
	@echo "========================="
	@echo "make         - Build project"
	@echo "make run     - Build and run (default: 100 users, 5000 txns)"
	@echo "make clean   - Remove build artifacts"
	@echo ""
	@echo "Usage:"
	@echo "  ./bin/fraud_detector [users] [transactions]"
	@echo "  ./bin/fraud_detector 50 1000"
	@echo "  ./bin/fraud_detector 500 50000"
