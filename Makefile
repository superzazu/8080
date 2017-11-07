TARGET = i8080_tests
LIBS =
CC = clang
CFLAGS = -g -Wall -O3
.PHONY: default all clean

default: $(TARGET)
all: default

SOURCES = $(shell find . -name '*.c')
HEADERS = $(shell find . -name '*.h')
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
