#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#define ARRAY_SIZE 10000

typedef struct {
    int first;
	int last;
} indices;

int main(int argc, char *argv[]) {
	nt arr[ARRAY_SIZE];
    srand(time(NULL));

	for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = rand() % 100000;
    }
    
    printf("First 10 elements of the array:\n");
	for (int i = 0; i < 10; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
	
	indices *set1 = (indices *)param;
	set1->first = 0;
	set1->last = (int) ARRAY_SIZE/4;
	
	indices *set2 = (indeces *)param;
	set2->first = (int) ARRAY_SIZE/4 + 1;
	set2->last = (int) ARRAY_SIZE/2;
	
	indices *set3 = (indices *)param;
	set3->first = (int) ARRAY_SIZE/2 + 1;
	set3->last = (int) ARRAY_SIZE*0.75;
	
	indices *set4 = (indices *)param;
	set4->first = (int) ARRAY_SIZE*0.75 + 1;
	set4->last = ARRAY_SIZE - 1;
    
	pthread_t thread0, thread1, thread2, thread3;
	pthread_create(&thread0, NULL, mergesort, NULL);
	pthread_create(&thread1, NULL, mergesort, NULL);
	pthread_create(&thread2, NULL, mergesort, NULL);
	pthread_create(&thread3, NULL, mergesort, NULL);
	
	pthread_join(thread0, NULL);
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
  
    int arr[] = { 4, 2, 5, 3, 1 };
    int n = sizeof(arr) / sizeof(arr[0]);

    quickSort(arr, 0, n - 1);

    pthread_t thread4, thread5, thread6;
	pthread_create(&thread4, NULL, mergesort, NULL);
	pthread_create(&thread5, NULL, mergesort, NULL);
	pthread_create(&thread6, NULL, mergesort, NULL);
	
	pthread_join(thread4, NULL);
	pthread_join(thread5, NULL);
	pthread_join(thread6, NULL);

    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);

    return 0;
}

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high) {

    // Initialize pivot to be the first element
    int p = arr[low];
    int i = low;
    int j = high;

    while (i < j) {

        // Find the first element greater than
        // the pivot (from starting)
        while (arr[i] <= p && i <= high - 1) {
            i++;
        }

        // Find the first element smaller than
        // the pivot (from last)
        while (arr[j] > p && j >= low + 1) {
            j--;
        }
        if (i < j) {
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[low], &arr[j]);
    return j;
}

void quickSort(int arr[], int low, int high) {
    if (low < high) {

        // call partition function to find Partition Index
        int pi = partition(arr, low, high);

        // Recursively call quickSort() for left and right
        // half based on Partition Index
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}
