#!/usr/bin/make
.SUFFIXES:
.PHONY: all run pack clean
TAR = shell prog
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)
PCK = lab-2.zip

CFLAGS = -std=gnu11 -c -g -Os -Wall -Werror -MMD -MP

DEP = $(OBJ:%.o=%.d)
-include $(DEP)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

all: $(TAR)

prog: prog.o
	$(CC) -o $@ $^

shell: $(filter-out prog.o,$(OBJ))
	$(CC) -o $@ $^

run: all
	./shell

pack:
	zip $(PCK) $(SRC) $(wildcard *.h) Makefile

clean:
	$(RM) $(RMFILES) $(TAR) $(OBJ) $(DEP) $(PCK)
