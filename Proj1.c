#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000

void *runner(void *param);
void *merge(int arr[], int left, int mid, int right);
void *mergeSort(int arr[], int left, int right);


typedef struct {
	int first;
	int last;
} indices;

int main(int argc, char *argv[]) {
	nt arr[ARRAY_SIZE];
    	srand(time(NULL));

    	// Fill the array with random integers
    	for (int i = 0; i < ARRAY_SIZE; i++) {
        	arr[i] = rand() % 100000;
    	}

    	// Print the first 10 elements to verify
    	printf("First 10 elements of the array:\n");
    	for (int i = 0; i < 10; i++) {
        p	rintf("%d ", arr[i]);
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
}

//Runner
void *runner(void *param) {
	int arr[] = { 12, 11, 13, 5, 6, 7 };
    	int n = sizeof(arr) / sizeof(arr[0]);
    
      	// Sorting arr using mergesort
    	mergeSort(arr, 0, n - 1);

    	for (int i = 0; i < n; i++) {
        	printf("%d ", arr[i]);
	}
	pthread_exit(0);	
}

void merge(int arr[], int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Create temporary arrays
    int leftArr[n1], rightArr[n2];

    // Copy data to temporary arrays
    for (i = 0; i < n1; i++)
        leftArr[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        rightArr[j] = arr[mid + 1 + j];

    // Merge the temporary arrays back into arr[left..right]
    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if (leftArr[i] <= rightArr[j]) {
            arr[k] = leftArr[i];
            i++;
        }
        else {
            arr[k] = rightArr[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of leftArr[], if any
    while (i < n1) {
        arr[k] = leftArr[i];
        i++;
        k++;
    }

    // Copy the remaining elements of rightArr[], if any
    while (j < n2) {
        arr[k] = rightArr[j];
        j++;
        k++;
    }
}

// The subarray to be sorted is in the index range [left-right]
void mergeSort(int arr[], int left, int right) {
    if (left < right) {
      
        // Calculate the midpoint
        int mid = left + (right - left) / 2;

        // Sort first and second halves
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);

        // Merge the sorted halves
        merge(arr, left, mid, right);
    }
}
