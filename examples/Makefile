CC = clang
CFLAGS = --target=wasm32-unknown-unknown -O3 -nostdlib
LDFLAGS = -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined

all: hello_fb.wasm

hello_fb.wasm: hello_fb.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f hello_fb.wasm

.PHONY: all clean
