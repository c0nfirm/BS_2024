#!/usr/bin/make
.SUFFIXES:

CFLAGS = -c -Os -Wall -Werror

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

%: %.o
	$(CC) -o $@ $^

all: fork

run: all
	./fork

clean:
	$(RM) $(RMFILES) fork
