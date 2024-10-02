#include <lba.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

const int NUM_THREADS = 8;

int shared_counter = 0;
lba_t lock;

void* print(void* arg) {
    int id = *((int*) arg);

    lba_lock(&lock, id);
    
    int temp = shared_counter;

    sleep(1);

    temp++;
    shared_counter = temp;

    printf("Hello from Thread %d - Shared Counter: %d\n", id, shared_counter);
    
    lba_unlock(&lock, id);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    if (lba_init(&lock) != 0) {
        fprintf(stderr, "Error: lba_init\n");
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

    lba_destroy(&lock);

    return 0;
}

