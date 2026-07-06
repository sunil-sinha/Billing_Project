CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS ?= -Iinclude

BUILD_DIR := build
SRC := src/main.c src/procedures.c
OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))
BIN := $(BUILD_DIR)/fifteen_columns

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@

$(BUILD_DIR)/%.o: %.c include/fifteen_columns.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(BIN)
	$(BIN) 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150

clean:
	rm -rf $(BUILD_DIR)

