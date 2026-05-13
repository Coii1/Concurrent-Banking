CC = gcc
INCLUDES = -Iinclude
SOURCES = $(wildcard src/*.c)
TARGET = bin/bankdb

CFLAGS_COMMON = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2
CFLAGS_DEBUG = $(CFLAGS_COMMON) -g -O1 -fsanitize=thread

LDFLAGS = -pthread
LDFLAGS_DEBUG = -pthread -fsanitize=thread

.PHONY: all debug clean test

all: $(TARGET)

$(TARGET): $(SOURCES)
	@mkdir -p bin
	$(CC) $(CFLAGS_RELEASE) $(INCLUDES) $(SOURCES) -o $(TARGET) $(LDFLAGS)

debug:
	@mkdir -p bin
	$(CC) $(CFLAGS_DEBUG) $(INCLUDES) $(SOURCES) -o $(TARGET) $(LDFLAGS_DEBUG)

test: all
	./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_simple.txt --tick-ms=100
	./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_readers.txt --tick-ms=100
	./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_abort.txt --tick-ms=100
	./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_deadlock.txt --tick-ms=100
	./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_buffer.txt --tick-ms=100

clean:
	rm -rf bin/bankdb
