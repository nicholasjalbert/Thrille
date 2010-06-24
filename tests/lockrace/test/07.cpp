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
}

void * routine1(void * arg) {
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    x++;
}

void * routine2(void * arg) {
    pthread_mutex_lock(&mutex);
    x = 1;
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[]) {
    setup();
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    pthread_create(&child, NULL, routine1, NULL);
    pthread_create(&child2, NULL, routine1, NULL);
    pthread_join(child, NULL);
    pthread_join(child2, NULL);
    teardown();
}

