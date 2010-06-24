//test cond use after destroy

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
    pthread_cond_signal(&cond);
}


int main(int argc, char *argv[]) {
    setup();
    pthread_cond_signal(&cond);
    teardown();
}

