#!/usr/bin/make
.SUFFIXES:

CFLAGS = -c -Os -Wall -Werror

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

%: %.o
	$(CC) -o $@ $^

all: pipe

run: all
	./pipe

clean:
	$(RM) $(RMFILES) pipe
