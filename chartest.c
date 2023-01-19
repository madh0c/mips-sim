#include <stdio.h>

int main(void) {
    int a = 65533;
    __int16_t b = a;
    printf("%d\n", b);
    return 0;
}