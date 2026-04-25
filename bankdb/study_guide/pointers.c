#include <stdio.h>

int main() {
    int x = 10;
    int *p = &x;

    printf("x: %d\n", x);          // value in x: 10
    printf("&x: %p\n", &x);        // address of x
    printf(" p: %p\n", p);          // value in p (should match &x)
    printf("*p: %d\n", *p);        // value at address in p: 10

    *p = 20;                       // change value at address p points to
    printf("x now: %d\n", x);      // x is now 20

    return 0;
}