#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 8

int shared_counter = 0;
pthread_mutex_t lock;

void* print(void* arg) {
    int id = *((int*) arg);

    pthread_mutex_lock(&lock);
    
    int temp = shared_counter;

    sleep(1);

    temp++;
    shared_counter = temp;

    printf("Hello from Thread %d - Shared Counter: %d\n", id, shared_counter);
    
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    if (pthread_mutex_init(&lock, NULL) != 0) {
        fprintf(stderr, "Error: pthread_mutex_init\n");
        return 1;
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;

        if (pthread_create(&threads[i], NULL, print, &ids[i]) != 0) {
            fprintf(stderr, "Error: pthread_create\n");
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);

    return 0;
}

