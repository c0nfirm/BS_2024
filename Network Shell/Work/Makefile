#!/usr/bin/make
.SUFFIXES:
.PHONY: all run pack clean
.SILENT: run

TAR = libshared.a client server
PCK = lab-3.zip
LIB = libshared.a

CFLAGS = -std=gnu11 -c -g -Os -Wall -MMD -MP -I "lib"
LFLAGS = -pthread
AFLAGS = rcs

SHARED_SRC = $(wildcard lib/*.c)
SHARED_OBJ = $(SHARED_SRC:%.c=%.o)
SHARED_DEP = $(SHARED_SRC:%.c=%.d)

CLIENT_SRC = $(wildcard cli/*.c)
CLIENT_OBJ = $(CLIENT_SRC:%.c=%.o)
CLIENT_DEP = $(CLIENT_SRC:%.c=%.d)

SERVER_SRC = $(wildcard srv/*.c)
SERVER_OBJ = $(SERVER_SRC:%.c=%.o)
SERVER_DEP = $(SERVER_SRC:%.c=%.d)

-include $(SHARED_DEP) $(CLIENT_DEP) $(SERVER_DEP)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

client: $(CLIENT_OBJ) $(LIB)
	$(CC) -o $@ $^ $(LFLAGS)

server: $(SERVER_OBJ) $(LIB)
	@if [ -e .srv_pid ]; then kill `cat .srv_pid`; $(RM) $(RMFILES) .srv_pid; fi
	$(CC) -o $@ $^ $(LFLAGS)

libshared.a: $(SHARED_OBJ)
	$(AR) $(AFLAGS) $@ $^

all: $(TAR)

.srv_pid: server
	./server& echo $$! > $@

run: all .srv_pid
	./client

pack: clean
	zip -r $(PCK) cli srv lib Makefile -x "*/.*"

clean:
	@[ -e .srv_pid ] && kill `cat .srv_pid` || true
	$(RM) $(RMFILES) .srv_pid $(TAR) $(PCK) $(CLIENT_OBJ) $(CLIENT_DEP) $(SERVER_OBJ) $(SERVER_DEP) $(SHARED_OBJ) $(SHARED_DEP)
