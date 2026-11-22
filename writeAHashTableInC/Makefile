CC ?= gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

# macOS gue doang, CLT SDK-nya kebaruan buat linker-nya
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
XCODE_SDK := $(shell ls -d /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX*.sdk 2>/dev/null | sort -V | tail -1)
ifneq ($(XCODE_SDK),)
SDKFLAGS := -isysroot $(XCODE_SDK)
endif
endif

.PHONY: all hashtable benchmark test clean

all: hashtable benchmark

hashtable: build/hash_table

build/hash_table: src/main.c src/hash_table.c src/prime.c
	@mkdir -p build
	$(CC) $(CFLAGS) $(SDKFLAGS) $^ -o $@ -lm

benchmark: build/benchmark

build/benchmark: src/benchmark.c src/hash_table.c src/hash_table_chain.c src/prime.c
	@mkdir -p build
	$(CC) $(CFLAGS) -O2 $(SDKFLAGS) $^ -o $@ -lm

test: hashtable benchmark
	@bash test_hashtable.sh

clean:
	rm -f build/hash_table build/benchmark
