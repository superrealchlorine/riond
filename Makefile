CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -std=c99
PREFIX ?= /usr/local

all: riond

riond: riond.c
	$(CC) $(CFLAGS) -o $@ $<

examples:
	$(MAKE) -C examples

install: riond
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 riond $(DESTDIR)$(PREFIX)/bin/riond

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/riond

clean:
	rm -f riond
	$(MAKE) -C examples clean

.PHONY: all examples install uninstall clean
