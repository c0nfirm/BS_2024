#!/usr/bin/make
.SUFFIXES:
.PHONY: all run pack clean

SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)
TAR = threadpool
PCK = lab-4.zip

CFLAGS = -std=gnu11 -c -g -Os -Wall -MMD -MP
LFLAGS = -pthread

DEP = $(OBJ:%.o=%.d)
-include $(DEP)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(TAR): $(filter-out quicksort.o,$(OBJ))
	$(CC) $(LFLAGS) -o $@ $^

all: $(TAR)

bench: $(filter-out main.o,$(OBJ))
	$(CC) $(LFLAGS) -o $@ $^

run: all
	./$(TAR)

pack: clean
	zip $(PCK) $(SRC) $(wildcard *.h) $(wildcard *.pdf) $(wildcard *.txt) Makefile

clean:
	$(RM) $(RMFILES) $(OBJ) $(TAR) bench $(DEP) $(PCK)
