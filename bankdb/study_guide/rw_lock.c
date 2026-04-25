// # Reader-Writer Lock Implementation in C with Semaphores

// This program demonstrates a reader-writer lock implementation using semaphores in C. It allows multiple concurrent readers, blocks writers while readers are active, and simulates writer starvation.

// ### Includes

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_READERS 5

sem_t mutex, writeblock;
int readcount = 0;

void *reader(void *arg) {
    int f = *(int *)arg;
    printf("Reader %d is trying to enter.\n", f);
    sem_wait(&mutex);
    readcount++;
    if (readcount == 1) {
        sem_wait(&writeblock);
    }
    sem_post(&mutex);

    // Reading section
    printf("Reader %d is reading.\n", f);
    sleep(1);

    sem_wait(&mutex);
    readcount--;
    if (readcount == 0) {
        sem_post(&writeblock);
    }
    sem_post(&mutex);
    return NULL;
}

void *writer(void *arg) {
    int f = *(int *)arg;
    printf("Writer %d is trying to enter.\n", f);
    sem_wait(&writeblock);

    // Writing section
    printf("Writer %d is writing.\n", f);
    sleep(2);
    printf("Writer %d has finished writing.\n", f);

    sem_post(&writeblock);
    return NULL;
}

int main() {
    pthread_t read[5], write[3];
    sem_init(&mutex, 0, 1);
    sem_init(&writeblock, 0, 1);

    // Creating reader threads
    for (int i = 0; i < 5; i++) {
        int *arg = malloc(sizeof(int));
        *arg = i;
        pthread_create(&read[i], NULL, reader, arg);
    }

    // Creating writer threads
    for (int i = 0; i < 3; i++) {
        int *arg = malloc(sizeof(int));
        *arg = i;
        pthread_create(&write[i], NULL, writer, arg);
    }

    // Joining reader threads
    for (int i = 0; i < 5; i++) {
        pthread_join(read[i], NULL);
    }

    // Joining writer threads
    for (int i = 0; i < 3; i++) {
        pthread_join(write[i], NULL);
    }

    printf("All readers and writers have finished.\n");
    return 0;
}

// gcc -o rw_lock rw_lock.c -pthread
// ./rw_lock