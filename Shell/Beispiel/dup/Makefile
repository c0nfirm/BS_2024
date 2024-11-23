#!/usr/bin/make
.SUFFIXES:

CFLAGS = -c -Os -Wall -Werror

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

%: %.o
	$(CC) -o $@ $^

all: dup

run: all
	./dup

clean:
	$(RM) $(RMFILES) dup
