# Concurrent Banking (CMSC 125 Lab 3)

## Flowchart

![flowchart](flowchart.JPG)

## Build

From bankdb/ directory:

	make clean
	make

Debug build (ThreadSanitizer):

	make debug

Run all provided tests:

	make test

## Usage

Required flags:

	./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_simple.txt --deadlock=prevention --tick-ms=100

Optional flags:

- --verbose (print per-tick logs)

## Features Implemented

- Multi-threaded transaction execution with timer thread
- Per-account reader-writer locks
- Deadlock prevention via lock ordering
- Bounded buffer pool with semaphores
- Final report with conservation check and transaction status

## Known Limitations

- Deadlock detection strategy is not implemented (prevention only)
- Account lookup is linear by account_id

## Run Tests (manual)

	./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_simple.txt --deadlock=prevention --tick-ms=100
	./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_readers.txt --deadlock=prevention --tick-ms=100
	./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_abort.txt --deadlock=prevention --tick-ms=100
	./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_deadlock.txt --deadlock=prevention --tick-ms=100
	./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_buffer.txt --deadlock=prevention --tick-ms=100