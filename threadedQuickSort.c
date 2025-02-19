#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define ARRAY_SIZE 10000
#define THREAD_COUNT 4

typedef struct {
    int *arr;
    int low;
    int high;
} indices;

int arr[ARRAY_SIZE]; // Global array shared beween threads

// Quicksort Code ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
int partition(int arr[], int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return i + 1;
}
void quickSort(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// Thread related code -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void* quickSortThread(void* arg) {
    indices *data = (indices*)arg;
    quickSort(data->arr, data->low, data->high);
    pthread_exit(NULL);
}
void merge(int low1, int high1, int low2, int high2) {
    int temp[ARRAY_SIZE];
    int i = low1, j = low2, k = 0;

    while (i <= high1 && j <= high2) {
        if (arr[i] < arr[j])
            temp[k++] = arr[i++];
        else
            temp[k++] = arr[j++];
    }
    
    while (i <= high1) temp[k++] = arr[i++];
    while (j <= high2) temp[k++] = arr[j++];

    for (i = 0, j = low1; j <= high2; i++, j++) {
        arr[j] = temp[i];
    }
}

int main() {
    pthread_t threads[THREAD_COUNT];
    indices thread_data[THREAD_COUNT];

    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) { // Create random array of array size
        arr[i] = rand() % 100000;
    }
    int partitionSize = ARRAY_SIZE / THREAD_COUNT; // Partition array into how many threads there are

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);  // Start timer

    for (int i = 0; i < THREAD_COUNT; i++) { // Create all threads that quicksort their partition
        thread_data[i].arr = arr;
        thread_data[i].low = i * partitionSize;
        thread_data[i].high = (i == THREAD_COUNT - 1) ? (ARRAY_SIZE - 1) : ((i + 1) * partitionSize - 1);
        pthread_create(&threads[i], NULL, quickSortThread, &thread_data[i]);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 1; i < THREAD_COUNT; i++) {
        merge(0, thread_data[i - 1].high, thread_data[i].low, thread_data[i].high);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);  // End timer
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Elapsed time: %f seconds\n", elapsed);

    return 0;
}
