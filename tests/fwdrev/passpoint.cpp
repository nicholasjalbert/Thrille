#include <errno.h>
#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

int x;
pthread_t child;
pthread_mutex_t mutex;

void setup() {
    pthread_mutex_init(&mutex, NULL);
}

void teardown() {
    pthread_mutex_destroy(&mutex);
}

void * routine1(void * arg) {
    pthread_mutex_lock(&mutex);
    x += 5;
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[]) {
    setup();
    x = 0;
    pthread_create(&child, NULL, routine1, NULL);
    while (x < 2) {
        pthread_mutex_lock(&mutex);
        x++;
        pthread_mutex_unlock(&mutex);
    }
    printf("x is %d\n", x);
    pthread_join(child, NULL);
    teardown();
}

