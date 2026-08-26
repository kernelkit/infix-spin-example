CC     ?= cc
CFLAGS ?= -O2 -Wall
LIBS    = -lsysrepo -lyang

all: hello-world

hello-world: hello-world.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f hello-world

.PHONY: all clean
