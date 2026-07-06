CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS ?= -Iinclude
LDFLAGS ?=

BUILD_DIR := build
SRC := $(wildcard src/core/*.c) \
       $(wildcard src/pipeline/*.c) \
       $(wildcard src/storage/*.c) \
       $(wildcard src/net/*.c) \
       $(wildcard src/cli/*.c)
OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))
BIN := $(BUILD_DIR)/edgeflow

TEST_SRC := $(filter-out src/cli/main.c,$(SRC)) $(wildcard tests/*.c)
TEST_OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_SRC))
TEST_BIN := $(BUILD_DIR)/edgeflow_tests

.PHONY: all clean test dirs

all: $(BIN)

dirs:
	@mkdir -p $(BUILD_DIR)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(TEST_BIN): $(TEST_OBJ)
	$(CC) $(TEST_OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c include/edgeflow.h | dirs
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test: $(TEST_BIN)
	$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)

