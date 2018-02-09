CC = gcc
CFLAGS = -Wall -pedantic -std=iso9899:1999 -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wconversion -lgmp

TARGETS = bmp_to_prime
DEBUG = -DDEBUG=0 -g

all: $(TARGETS)

debug: DEBUG = -DDEBUG=1 -g
debug: all

$(TARGETS): %: %.c
	$(CC) $(CFLAGS) $(DEBUG) $< -o $@

clean:
	$(RM) $(TARGETS)
