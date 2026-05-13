# Flowchart

![flowchart](flowchart.JPG)

# Run Tests
# From bankdb/ directory:

# Simple trace - basic operations test
./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_simple.txt --tick-ms=100

# Reader-heavy workload
./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_readers.txt --tick-ms=100

# Abort/rollback scenario
./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_abort.txt --tick-ms=100

# Deadlock prevention test
./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_deadlock.txt --tick-ms=100

# Buffer pool stress test
./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_buffer.txt --tick-ms=100