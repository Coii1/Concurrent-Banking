CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -O1 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -pthread
INCLUDES = -Iinclude

SOURCES = $(wildcard src/*.c)
TARGET = bin/bankdb

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -rf bin/bankdb
