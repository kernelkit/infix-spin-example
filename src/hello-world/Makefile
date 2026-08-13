CC     ?= cc
CFLAGS ?= -O2 -Wall

all: hello-world

hello-world: hello-world.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f hello-world

.PHONY: all clean
