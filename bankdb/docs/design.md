# Design Notes

This document should explain architectural decisions, concurrency strategy, synchronization choices, and tradeoffs.


To run:
    cd bankdb
    gcc -Wall -Wextra -std=c11 -Iinclude src/*.c -o bin/bankdb -pthread
    ./bin/bankdb tests/accounts.txt tests/trace_simple.txt



Note: current cli interface is 
./bin/bankdb tests/accounts.txt tests/trace_simple.txt

instead of $ ./bankdb --accounts=accounts.txt --trace=trace.txt \
        --deadlock=prevention --tick-ms=100

for simplcity and initial development.
this is for the sake if simplicity for now
TODO: implement proper CLI parsing with --accounts= 

account creation is inside bank creation directly to avoid memory leaks of separately allocating an Account struct
Having another load_accounts function separately allocating memory for accounts and returning those account requires 
freeing every allocated account slot 
TODO: consult with sir Ren which is better


Transactions pointers array is currently initialized in main, NOT SURE if it should be added in the bank struct. 
Also, for each transaction operation, a start tick field is added to the operation to only wake the operation 
that need to wake at a tick insted of them racing or just running sequentialy when the transaction is waked. 
TODO: consult where transactions should be initialized.
