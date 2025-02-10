#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000

void *mergesort(void *param);

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

void *mergesort(void *param) {
	//Enter code here
	}
	pthread_exit(0);	
}
