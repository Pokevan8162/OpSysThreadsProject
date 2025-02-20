// Evan Stoller, Ben Kuehner, Ryan Junod
// Average computation time: 0.0642516 seconds (10 test runs, 0.0631003 seconds slower on average than single-threaded quick sort)
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>

#define ARRAY_SIZE 10000
#define THREAD_COUNT 4

typedef struct {
    int *arr;
    int low;
    int high;
} indices;

int arr[ARRAY_SIZE];

// Quicksort Code ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

// Thread related code -----------------------------------------------------------------------------------------------------------------------------------------------------------------------

void* quickSortThread(void* arg) {
    indices *data = (indices*)arg;
    quickSort(data->arr, data->low, data->high);
    pthread_exit(NULL);
}

// Thread partitions array and returns pivot
void* threadPartition(void* arg) {
    indices *data = (indices*)arg;
    int pivot = partition(data->arr, data->low, data->high);
    return (void*)(intptr_t)pivot;  // Safe casting
}

// Main program ------------------------------------------------------------------------------------------------------------------------------------------------------------------
int main() {
    pthread_t threads[THREAD_COUNT];
    indices thread_data[THREAD_COUNT];

    // Create the array and set up the time clock
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) { // Create random array of array size
        arr[i] = rand() % 100000;
    }
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);  // Start timer

    // Create two threads to partition the 2 partitions
    int pivot = partition(arr, 0, ARRAY_SIZE - 1);
    for (int i = 0; i < 2; i++) {
        thread_data[i].arr = arr;
        thread_data[i].low = i * pivot;
        thread_data[i].high = (i == 0) ? (pivot) : (ARRAY_SIZE - 1);
        pthread_create(&threads[i], NULL, threadPartition, &thread_data[i]);
    }

    // After the two threads are done, create 4 threads that will sort the rest of the array
    void* newPivot;
    pthread_join(threads[0], &newPivot);
    int nextPivot = (int)(intptr_t)newPivot;  // Safe casting

    for (int i = 0; i < 2; i++) {
        thread_data[i].arr = arr;
        thread_data[i].low = i * (nextPivot + 1); 
        thread_data[i].high = (i == 0) ? (nextPivot - 1) : (pivot - 1);
        pthread_create(&threads[i], NULL, quickSortThread, &thread_data[i]);
    }

    pthread_join(threads[1], &newPivot);
    nextPivot = (int)(intptr_t)newPivot;

    for (int i = 2; i < 4; i++) {
        thread_data[i].arr = arr;
        thread_data[i].low = (i == 2) ? (pivot + 1) : (nextPivot + 1);
        thread_data[i].high = (i == 2) ? (nextPivot - 1) : (ARRAY_SIZE - 1);
        pthread_create(&threads[i], NULL, quickSortThread, &thread_data[i]);
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);  // End timer
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Elapsed time: %f seconds\n", elapsed);

    return 0;
}
