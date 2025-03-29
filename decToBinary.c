#include <stdio.h>
#include <stdlib.h>

int decToBinary(int decimal) {
    int binary = 0;
    int place = 1;

    while (decimal > 0) {
        int remainder = decimal % 2;
        binary += remainder * place;
        place *= 10;
        decimal /= 2;
    }

    return binary;
}

int main() {
    int binaryNumber = decToBinary(990);
    printf("Binary: %d\n", binaryNumber);

    return 0;
}
