// Evan Stoller, Ben Kuehner, Ryan Junod
// Average computation time: 0.0011513 seconds (10 test runs)
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define ARRAY_SIZE 10000

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high) {
    int p = arr[low];
    int i = low;
    int j = high;

    while (i < j) {
        while (arr[i] <= p && i <= high - 1) i++;
        while (arr[j] > p && j >= low + 1) j--;
        if (i < j) swap(&arr[i], &arr[j]);
    }
    swap(&arr[low], &arr[j]);
    return j;
}

void quickSort(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

void* runQuicksort(void* arg) {
    struct timespec start, end;
    int arr[ARRAY_SIZE];
    srand(time(NULL));

    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = rand() % 100000;
    }

    int n = sizeof(arr) / sizeof(arr[0]);

    clock_gettime(CLOCK_MONOTONIC, &start);  // Start timer
    quickSort(arr, 0, n - 1);
    clock_gettime(CLOCK_MONOTONIC, &end);  // End timer

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;  // Calculate time elapsed
    printf("Elapsed time: %f seconds\n", elapsed);

    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t singleThread; // Creates a single thread to enforce only one threads functionality during processing
    pthread_create(&singleThread, NULL, runQuicksort, NULL);
    pthread_join(singleThread, NULL);

    return 0;
}
