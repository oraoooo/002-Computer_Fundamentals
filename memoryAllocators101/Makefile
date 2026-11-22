CC ?= gcc

# macOS gue aja, CLT SDK-nya kebaruan buat linker-nya
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
XCODE_SDK := $(shell ls -d /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX*.sdk 2>/dev/null | sort -V | tail -1)
ifneq ($(XCODE_SDK),)
SDKFLAGS := -isysroot $(XCODE_SDK)
endif
endif

.PHONY: all memalloc test clean

all: memalloc memalloc_test

# Linux: LD_PRELOAD=$(PWD)/memalloc.so <program>
memalloc: memalloc.so

memalloc.so: memalloc.c
	$(CC) $(SDKFLAGS) -o $@ -fPIC -shared $<

# link langsung ke binary test, ga pake LD_PRELOAD - biar jalan sama di macOS
memalloc_test: memalloc.c test_malloc.c
	$(CC) -Wall -Wextra $(SDKFLAGS) -o $@ $^

test: memalloc_test
	@bash test_malloc.sh

clean:
	rm -f memalloc.so memalloc_test
