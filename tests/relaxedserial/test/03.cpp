//try to schedule thread 1 too many times, should auto switch to thread 0
//when thread 1 becomes disabled
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

void builtintest() {
    printf("My return addr %p\n", __builtin_return_address(0));
}

void setup() {
    pthread_mutex_init(&mutex, NULL);
}

void teardown() {
    pthread_mutex_destroy(&mutex);
}

void * routine1(void * arg) {
    pthread_mutex_lock(&mutex);
    x = 1;
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[]) {
    setup();
    builtintest();
    pthread_create(&child, NULL, routine1, NULL);
    pthread_mutex_lock(&mutex);
    x = 3;
    pthread_mutex_unlock(&mutex);
    printf("x is %d\n", x);
    pthread_join(child, NULL);
    teardown();
}

