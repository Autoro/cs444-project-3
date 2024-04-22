CC=gcc
CCOPTS=-Wall -Wextra -Werror -g
LIBS=-lpthread

SRCS=$(wildcard *.c)

.PHONY: all clean

all: pc

clean:
	rm -f pc

pc: $(SRCS)
	$(CC) $(CCOPTS) -o $@ $^ $(LIBS)
