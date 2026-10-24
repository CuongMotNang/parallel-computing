# Build các chương trình Parallel Prefix Sum (chạy trong Linux/WSL)
CC      ?= gcc
CFLAGS  ?= -O3 -march=native -std=gnu11 -Wall -Wextra -fopenmp
BUILD   := build

all: $(BUILD)/bench $(BUILD)/test_scan $(BUILD)/compact

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/bench: src/bench.c src/scan.c src/scan.h | $(BUILD)
	$(CC) $(CFLAGS) src/bench.c src/scan.c -o $@

$(BUILD)/test_scan: src/test_scan.c src/scan.c src/scan.h | $(BUILD)
	$(CC) $(CFLAGS) src/test_scan.c src/scan.c -o $@

$(BUILD)/compact: src/compact.c src/scan.c src/scan.h | $(BUILD)
	$(CC) $(CFLAGS) src/compact.c src/scan.c -o $@

test: $(BUILD)/test_scan
	./$(BUILD)/test_scan

clean:
	rm -rf $(BUILD)

.PHONY: all test clean
