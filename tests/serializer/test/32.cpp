//test barrier 
// deadlock on success

#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

int x;
pthread_t child, child2;
pthread_mutex_t mutex, mutex2;
pthread_cond_t cond, cond2;
pthread_barrier_t bar;

void setup() {
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&cond2, NULL);
}

void teardown() {
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex2);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond2);
}

void * routine1(void * arg) {
    pthread_barrier_wait(&bar);
}


int main(int argc, char *argv[]) {
    setup();
    pthread_barrier_init(&bar, NULL, 2);
    pthread_create(&child, NULL, routine1, NULL);
    pthread_join(child, NULL);
    pthread_join(child2, NULL);
    teardown();
}

