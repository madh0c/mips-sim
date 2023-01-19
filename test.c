#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


int main(void) {
    int hex = 0x3402000B;
    
    if ((hex >> 26) == 5) {
        printf("bad\n");
    }

    
    return 0;
}
