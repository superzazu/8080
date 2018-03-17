BIN = i8080_tests
CC = cc
CFLAGS = -g -Wall -Wextra -O3 -std=c11 -pedantic
LDFLAGS =

SOURCES = $(shell find . -name '*.c')
OBJECTS = $(SOURCES:.c=.o)

.PHONY: clean

default: $(BIN)

$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJECTS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BIN) *.o
