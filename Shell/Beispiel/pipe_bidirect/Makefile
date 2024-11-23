#!/usr/bin/make
.SUFFIXES:

CFLAGS = -c -Os -Werror

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

%: %.o
	$(CC) -o $@ $^

all: pingpong

run: all
	@echo "writing into log.txt; stop execution with CTRL+C"
	./pingpong

clean:
	$(RM) $(RMFILES) pingpong
