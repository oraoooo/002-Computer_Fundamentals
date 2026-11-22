CC ?= gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

# macOS gue aja yg CLT SDK-nya kebaruan buat linker-nya, jadi kalo ketemu
# Xcode.app pake SDK dia aja. Kalo gaada ya SDKFLAGS kosong, normal aja.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
XCODE_SDK := $(shell ls -d /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX*.sdk 2>/dev/null | sort -V | tail -1)
ifneq ($(XCODE_SDK),)
SDKFLAGS := -isysroot $(XCODE_SDK)
endif
endif

.PHONY: kilo test clean

kilo: kilo.c
	$(CC) $(CFLAGS) $(SDKFLAGS) kilo.c -o kilo

test: kilo
	@python3 test_kilo.py

clean:
	rm -f kilo
