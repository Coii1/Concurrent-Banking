#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    printf("Program name: %s\n", argv[0]);
    printf("Total arguments: %d\n\n", argc);
    
    if (argc < 2) {
        printf("Usage: %s <arg1> [arg2] [arg3] ...\n", argv[0]);
        printf("Example: %s hello world 123\n", argv[0]);
        return 1;
    }
    
    // Print all arguments
    printf("Arguments:\n");
    for (int i = 1; i < argc; i++) {
        printf("  argv[%d]: %s\n", i, argv[i]);
    }
    
    // Example: convert first numeric argument to integer
    if (argc > 1) {
        int num = atoi(argv[1]);
        printf("\nFirst argument as number: %d\n", num);
    }
    
    return 0;
}
