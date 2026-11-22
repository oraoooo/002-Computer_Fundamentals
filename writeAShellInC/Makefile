CC ?= gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

# fix buat macOS gue doang: CLT SDK-nya kebaruan, linker lama gak ngerti.
# kalo ga ketemu Xcode.app ya SDKFLAGS kosong, gcc normal aja.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
XCODE_SDK := $(shell ls -d /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX*.sdk 2>/dev/null | sort -V | tail -1)
ifneq ($(XCODE_SDK),)
SDKFLAGS := -isysroot $(XCODE_SDK)
endif
endif

.PHONY: all test clean

all: main

main: writeAShellInC.c
	$(CC) $(CFLAGS) $(SDKFLAGS) writeAShellInC.c -o main

test: main
	@bash test_shell.sh

clean:
	rm -f main
