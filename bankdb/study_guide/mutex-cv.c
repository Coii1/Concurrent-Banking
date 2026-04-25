#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

pthread_mutex_t mutexFuel;
pthread_cond_t condFuel;
int fuel = 0;

void* fuel_filling(void* arg) {
    for (int i = 0; i < 5; i++) {
        pthread_mutex_lock(&mutexFuel);
        fuel += 60;
        printf("Filled fuel... %d\n", fuel);
        pthread_mutex_unlock(&mutexFuel);
        pthread_cond_broadcast(&condFuel);
        // signal wakes one waiting thread, if there is one. Broadcast wakes all waiting threads. 
        sleep(1);
        /*
        Without sleep, the fuel filling thread will fill fuel 5 times before any of the car threads get a chance to run.
        after signal, The car starts slowly waking up (this takes a few microseconds for the OS to do context switching).
        Before the car can even stand up and grab the lock, the fuel_filling thread loops back around and snatches the lock again!

        This is called Thread Starvation. The filler thread is so aggressive that it keeps stealing the lock back before the cars have a chance to take it.
        */
    }
}

void* car(void* arg) {
    pthread_mutex_lock(&mutexFuel);
    while (fuel < 40) {
        printf("No fuel. Waiting...\n");
        pthread_cond_wait(&condFuel, &mutexFuel);
        // Equivalent to:
        // pthread_mutex_unlock(&mutexFuel);
        // wait for signal on condFuel
        // pthread_mutex_lock(&mutexFuel);
    }
    fuel -= 40;
    printf("Got fuel. Now left: %d\n", fuel);
    pthread_mutex_unlock(&mutexFuel);
}

int main(int argc, char* argv[]) {
    pthread_t th[5];
    pthread_mutex_init(&mutexFuel, NULL);
    pthread_cond_init(&condFuel, NULL);
    for (int i = 0; i < 5; i++) {
        if (i == 4) {
            if (pthread_create(&th[i], NULL, &fuel_filling, NULL) != 0) {
                perror("Failed to create thread");
            }
        } else {
            if (pthread_create(&th[i], NULL, &car, NULL) != 0) {
                perror("Failed to create thread");
            }
        }
    }

    for (int i = 0; i < 5; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }
    pthread_mutex_destroy(&mutexFuel);
    pthread_cond_destroy(&condFuel);
    return 0;
}


/*
No fuel. Waiting...
No fuel. Waiting...
No fuel. Waiting...
No fuel. Waiting...
Filled fuel... 15
Filled fuel... 30
Filled fuel... 45
Filled fuel... 60
Got fuel. Now left: 20
No fuel. Waiting...
No fuel. Waiting...
Filled fuel... 35
No fuel. Waiting...
No fuel. Waiting...

due to:
for (int i = 0; i < 5; i++) {
    fuel += 15;
}
// not enough fuel.
*/