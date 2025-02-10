#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

int pointsInside = 0;
int totalPoints = 0;

void *runner(void *param);

int main(int argc, char *argv[]) {
	pthread_t thread0, thread1, thread2, thread3, thread4;
	pthread_create(&thread0, NULL, runner, NULL);
	pthread_create(&thread1, NULL, runner, NULL);
	pthread_create(&thread2, NULL, runner, NULL);
	pthread_create(&thread3, NULL, runner, NULL);
	pthread_create(&thread4, NULL, runner, NULL);
	
	pthread_join(thread0, NULL);
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	pthread_join(thread3, NULL);
	
	float pi = (float) 4 * pointsInside / totalPoints;
	printf("%.5f\n", pi);
	return 0;
}

//generates 1000000 points, saving data in global variables
void *runner(void *param) {
	srand(time(NULL));
	float x, y;
	for (int i = 0; i<1000000; i++) {
		x = (float) rand() / RAND_MAX;
		y = (float) rand() / RAND_MAX;
		//Point is in the circle if the distance to center (0.5, 0.5) is within 0.5
		float distance = sqrt(pow(x-0.5, 2) + pow(y-0.5, 2));
		if (distance <= 0.5) {
			pointsInside++;
		}
		totalPoints++;
	}
	pthread_exit(0);	
}
