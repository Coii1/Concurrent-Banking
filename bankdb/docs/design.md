# Design Notes

This document should explain architectural decisions, concurrency strategy, synchronization choices, and tradeoffs.


To run:
    cd bankdb
    gcc -Wall -Wextra -std=c11 -Iinclude src/*.c -o bin/bankdb -pthread 
    //other way: gcc -Wall -Wextra -std=c11 -g -O1 -Iinclude src/*.c -o bin/bankdb -pthread -fsanitize=thread
    //other way: gcc -Wall -Wextra -std=c11 -Iinclude src/*.c -o bin/bankdb -pthread -fsanitize=thread
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
TODO: consult with sir Ren which is better // done, it's actually prefered in this case.


Transactions pointers array is currently initialized in main, NOT SURE if it should be added in the bank struct. 
Also, for each transaction operation, a start tick field is added to the operation to only wake the operation 
that need to wake at a tick insted of them racing or just running sequentialy when the transaction is waked. 
TODO: consult where transactions should be initialized. //done: Ok for now


Bank instance is made global for now so that accounts can be accessed in transactions.c 
this is temporary since bufferpool isn't implemented yet. 


after implementing the TRANSFER function in the transaction thread, it is evident that given millions of accounts in a bank, having 
to search for the index given the id is very inefficient.we should have a more efficient way to map account_id to its index in the 
accounts array, such as a hash map or a direct mapping if account ids are guaranteed to be dense and within a certain range. for now,
we will just do a linear search to find the indices of the from and to accounts, but this is a clear area for optimization in the future.


issues found
1. currently since global tick is initialized to 0 from the start, there are times when a transaction races first with the clock thread. so it executes first. 