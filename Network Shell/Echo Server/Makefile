#!/usr/bin/make
.SUFFIXES:
.PHONY: all run clean
.SILENT: run

TAR = client server
CFLAGS = -c -Os -Wall -Werror

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

%: %.o
	$(CC) -o $@ $^

all: $(TAR)

run: all
	./server&
	./client

clean:
	$(RM) $(RMFILES) $(TAR) *.o
