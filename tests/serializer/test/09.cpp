//test cond_broadcast

#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

int x;
bool y;
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
}

void * routine1(void * arg) {
    pthread_mutex_lock(&mutex);
    while (!y) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void * routine2(void * arg) {
    pthread_mutex_lock(&mutex);
    y = true;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}


int main(int argc, char *argv[]) {
    setup();
    y = false;
    pthread_create(&child, NULL, routine1, NULL);
    pthread_create(&child2, NULL, routine2, NULL);
    pthread_mutex_lock(&mutex);
    while (!y) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    pthread_join(child, NULL);
    pthread_join(child2, NULL);
    teardown();
}

