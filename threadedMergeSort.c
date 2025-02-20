// Evan Stoller, Ben Kuehner, Ryan Junod
// Average computation time: 0.0010178 seconds (10 test runs, 0.0006932 seconds faster on average than single-threaded merge sort)
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define ARRAY_SIZE 10000
#define THREAD_COUNT 4

typedef struct { // Stores the indices for use of the threads to know what parts of the array to copy
    int left;
    int right;
} indices;

int arr[ARRAY_SIZE]; // Instantiate array of predefined ARRAY_SIZE value

void merge(int arr[], int left, int mid, int right) { // Normal merge function
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int leftArr[n1], rightArr[n2];

    for (i = 0; i < n1; i++)
        leftArr[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        rightArr[j] = arr[mid + 1 + j];

    i = 0; 
    j = 0; 
    k = left;
    
    while (i < n1 && j < n2) {
        if (leftArr[i] <= rightArr[j]) {
            arr[k++] = leftArr[i++];
        } else {
            arr[k++] = rightArr[j++];
        }
    }

    while (i < n1) arr[k++] = leftArr[i++];
    while (j < n2) arr[k++] = rightArr[j++];
}

void mergeSort(int arr[], int left, int right) { // Normal merge sort function
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void* mergeSortThread(void* arg) { // Threads utilize this function to pass their indices to what they are supposed to merge sort. Parameters are null so threads can input their own parameters
    indices *args = (indices*)arg;    // Get indices struct from parameters
    mergeSort(arr, args->left, args->right);    // Merge sort based on these index values from the struct
    pthread_exit(NULL);
}

void* mergeThread(void* arg) { // Threads utilize this function to merge the arrays back together
    indices *args = (indices*)arg;    // Get indices struct from parameters
    int left = args->left;    // Get the left value from the indices struct
    int right = args->right;    // Get the right value from the indices struct
    int mid = left + (right - left) / 2;

    merge(arr, left, mid, right);    // Merge based on those indices
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[THREAD_COUNT];    // Create as many threads that are specified in the THREAD_COUNT integer
    indices threadIndices[THREAD_COUNT]; // Carries the indices per thread

    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {    // Create random array
        arr[i] = rand() % 100000;
    }
    int partitionSize = ARRAY_SIZE / THREAD_COUNT;    // Partition size is created based on the array size divided by the amount of threads
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);    // Start time

    for (int i = 0; i < THREAD_COUNT; i++) { // Creates threads and puts them in the merge sort function
        threadIndices[i].left = i * partitionSize;    // Left index is i * partition size (ex. i = 0, left = 0)
        threadIndices[i].right = (i == THREAD_COUNT - 1) ? (ARRAY_SIZE - 1) : ((i + 1) * partitionSize - 1);    // Right index is set to array size - 1 if it is the last thread, or i+1 * partition size-1 otherwise.
        pthread_create(&threads[i], NULL, mergeSortThread, &threadIndices[i]);    // Creates the threads and has them run mergseSortThread, passing their indices as an object parameter
    }

    for (int i = 0; i < THREAD_COUNT; i++) { // Waits for the threads to finish sorting (does not affect how the threads finish, just waits until they all say theyre done starting by thread 0
        pthread_join(threads[i], NULL);
    }

    int partitionsRemaining = THREAD_COUNT; // This value is used later to dictate how many threads we need per merge
    while (partitionsRemaining > 1) { // While there are still arrays that need to be merged
        int mergeThreadCount = partitionsRemaining / 2; // We wont need all 4 threads to merge 4 subarrays (for ex), we'd only need two, so divide it by 2
        pthread_t mergeThreads[mergeThreadCount];    // Create the merging threads based on the count value just instantiated (not to be confused by the method mergeThread)
        indices mergeData[mergeThreadCount];    // Create indices for how many different merging threads there are

        for (int i = 0; i < mergeThreadCount; i++) {    // Parallel merging
            mergeData[i].left = threadIndices[i * 2].left;       // i*2 is used to use these threads to merge each partition in parallel. For example, when i=0, i*2 and i*2+1 returns 0 and 1, so a thread merges-
            mergeData[i].right = threadIndices[i * 2 + 1].right; // partitions 0 and 1, and the next iteration makes the next thread merge partitions 2 and 3, and so on.

            pthread_create(&mergeThreads[i], NULL, mergeThread, &mergeData[i]);
        }

        for (int i = 0; i < mergeThreadCount; i++) {    // Wait for all threads to finish
            pthread_join(mergeThreads[i], NULL);
        }

        partitionsRemaining /= 2;    // Divide partitionsRemaining by 2 because there are now partitions/2 partitions remaining to merge in order to dictate how many threads we need.
    }

    clock_gettime(CLOCK_MONOTONIC, &end);    // End time
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Elapsed time: %f seconds\n", elapsed);

    return 0;
}
