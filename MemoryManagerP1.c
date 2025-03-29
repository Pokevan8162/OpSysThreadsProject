#include <stdio.h>
#include <stdlib.h>

#define MAX_ADDRESSES 1000

int addresses[MAX_ADDRESSES];

// Used by TLB, stored in binary
typedef struct {
    int page;
    int frame;
} tlbPage;

// Function to read addresses from a text file
int readAddresses() {
    FILE *file;
    int value;
    int index = 0;

    file = fopen("addresses.txt", "r");
    if (file == NULL) {
        perror("Error opening addresses file");
        return 1;
    }

    // Read integer values from file
    while (fscanf(file, "%d", &value) == 1 && index < MAX_ADDRESSES) {
        addresses[index++] = value;
    }

    fclose(file);
    return 0;
}

// Function to read a binary file and return a dynamically allocated array
unsigned char* readBin(size_t *fileSize) {
    FILE *file;
    unsigned char *buffer;
    
    file = fopen("data.bin", "rb");
    if (!file) {
        perror("Error opening binary file");
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    rewind(file);

    // Allocate memory for file contents
    buffer = (unsigned char*) malloc(*fileSize);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    // Read file into buffer
    fread(buffer, 1, *fileSize, file);
    fclose(file);
    
    return buffer;
}

int main() {
    // Read addresses from the file
    if (readAddresses() == 0) {
        printf("Addresses successfully read from file.\n");
    }

    // Read binary file
    size_t fileSize;
    unsigned char *binaryData = readBin(&fileSize);
    if (binaryData) {
        printf("Binary file read successfully. Size: %zu bytes\n", fileSize);
        
        // Print binary data in hex format
        for (size_t i = 0; i < fileSize; i++) {
            printf("%02X ", binaryData[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");

        free(binaryData); // Free allocated memory
    }

    tlbPage[16] TLB;
    
    return 0;
}
