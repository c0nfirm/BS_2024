#!/usr/bin/make
.SUFFIXES:

CFLAGS = -c -O0

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

%: %.o
	$(CC) -o $@ $^

all: fork signal

run: all
	./fork 10
	./signal

clean:
	$(RM) $(RMFILES) fork signal
