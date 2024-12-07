#!/usr/bin/make
.SUFFIXES:
.PHONY: all run clean
TAR = server
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)

DEP = $(OBJ:%.o=%.d)
-include $(DEP)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(TAR): $(OBJ)
	$(CC) $(LFLAGS) $^ -o $@

all: $(TAR)

run: all
	./$(TAR)

clean:
	$(RM) $(RMFILES) $(OBJ) $(TAR) $(DEP)
