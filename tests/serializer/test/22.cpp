//test semaphore initialization (deadlock on fail)

#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>

using namespace std;

int x;
bool y;
sem_t sem;
pthread_t child, child2;
pthread_mutex_t mutex, mutex2;
pthread_cond_t cond, cond2;

void setup() {
    pthread_mutex_init(&mutex2, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&cond2, NULL);
    sem_init(&sem, 0, 1);
}

void teardown() {
    pthread_mutex_destroy(&mutex2);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond2);
}

void * routine(void * arg) {
}


int main(int argc, char *argv[]) {
    setup();
    pthread_mutexattr_t myattr;
    pthread_mutexattr_init(&myattr);
    pthread_mutexattr_settype(&myattr, PTHREAD_MUTEX_NORMAL);
    pthread_mutex_init(&mutex, &myattr);
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    pthread_mutexattr_settype(&myattr, PTHREAD_MUTEX_RECURSIVE);
    teardown();
}

