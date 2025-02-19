// Average computation time: 0.0010178 seconds (10 test runs, 0.0006932 seconds faster on average than merge sort)
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define ARRAY_SIZE 10000
#define THREAD_COUNT 4

typedef struct {
    int left;
    int right;
} indices;

int arr[ARRAY_SIZE];

void merge(int arr[], int left, int mid, int right) {
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

void mergeSort(int arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void* mergeSortThread(void* arg) { // What the threads use for their specific array indices
    indices *args = (indices*)arg;
    mergeSort(arr, args->left, args->right);
    pthread_exit(NULL);
}

void* mergeThread(void* arg) { // What the threads use to merge the arrays back together
    indices *args = (indices*)arg;
    int left = args->left;
    int right = args->right;
    int mid = left + (right - left) / 2;

    merge(arr, left, mid, right);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[THREAD_COUNT];
    indices threadIndices[THREAD_COUNT]; // Carries the indices per thread

    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) { 
        arr[i] = rand() % 100000;
    }

    int partitionSize = ARRAY_SIZE / THREAD_COUNT;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < THREAD_COUNT; i++) { // Creates threads and puts them in the merge sort function
        threadIndices[i].left = i * partitionSize;
        threadIndices[i].right = (i == THREAD_COUNT - 1) ? (ARRAY_SIZE - 1) : ((i + 1) * partitionSize - 1);
        pthread_create(&threads[i], NULL, mergeSortThread, &threadIndices[i]);
    }

    for (int i = 0; i < THREAD_COUNT; i++) { // Waits for the threads to finish sorting
        pthread_join(threads[i], NULL);
    }

    int mergePass = THREAD_COUNT; // Limit on how many merges are allowed to happen at once, up to how many threads we define it as
    while (mergePass > 1) { // While there are still arrays that need to be merged
        int mergeThreadCount = mergePass / 2; // We wont need all 4 threads to merge 4 subarrays (for ex), we'd only need two, so divide it by 2
        pthread_t mergeThreads[mergeThreadCount];
        indices mergeData[mergeThreadCount];

        for (int i = 0; i < mergeThreadCount; i++) {
            mergeData[i].left = threadIndices[i * 2].left;
            mergeData[i].right = threadIndices[i * 2 + 1].right;

            pthread_create(&mergeThreads[i], NULL, mergeThread, &mergeData[i]);
        }

        for (int i = 0; i < mergeThreadCount; i++) {
            pthread_join(mergeThreads[i], NULL);
        }

        mergePass /= 2;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Elapsed time: %f seconds\n", elapsed);

    return 0;
}
