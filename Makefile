CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
TARGET = riond
PREFIX = /usr/local

all: $(TARGET)

$(TARGET): riond.c
	$(CC) $(CFLAGS) -o $(TARGET) riond.c

clean:
	rm -f $(TARGET)

install: $(TARGET)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/

.PHONY: all clean install
