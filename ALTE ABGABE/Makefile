#!/usr/bin/make
.SUFFIXES:
.PHONY: all run pack clean
.SILENT: run

TAR = cli/client srv/server
PCK = lab-3.zip

export CFLAGS = -std=gnu11 -c -g -Os -Wall -Werror -MMD -MP

cli/client: $(wildcard cli/*.c) $(wildcard cli/*.h)
	@$(MAKE) -C $(@D)

srv/server: $(wildcard srv/*.c) $(wildcard srv/*.h)
	@$(MAKE) -C $(@D)

all: $(TAR)

run: all
	srv/server& echo $$! > .srv_pid
	echo "Client Ready:"
	cli/client
	kill `cat .srv_pid`
	$(RM) $(RMFILES) .srv_pid

pack: clean
	zip -r $(PCK) cli srv Makefile -x "*/.*"

clean:
	@$(MAKE) -C cli clean
	@$(MAKE) -C srv clean
	$(RM) $(RMFILES) $(PCK)
