//test cond_wait

#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

int x;
int * ptr;
pthread_t child, child2, child3, child4;
pthread_mutex_t mutex;

void setup() {
    pthread_mutex_init(&mutex, NULL);
}

void teardown() {
    pthread_mutex_destroy(&mutex);
}

void * routine1(void * arg) {
    pthread_mutex_lock(&mutex);
    ptr = NULL;
    pthread_mutex_unlock(&mutex);
    
}


void * routine2(void * arg) {
    pthread_mutex_lock(&mutex);
    x = 4;
    ptr = &x;
    pthread_mutex_unlock(&mutex);
    
}

void * routine3(void * arg) {
    pthread_mutex_lock(&mutex);
    x = 5;
    ptr = &x;
    pthread_mutex_unlock(&mutex);
    
}

void * routine4(void * arg) {
    pthread_mutex_lock(&mutex);
    x = 3;
    ptr = &x;
    pthread_mutex_unlock(&mutex);
    
}



int main(int argc, char *argv[]) {
    setup();
    x = 0;
    ptr = &x;
    pthread_create(&child, NULL, routine1, NULL);
    pthread_create(&child2, NULL, routine2, NULL);
    pthread_create(&child3, NULL, routine3, NULL);
    pthread_create(&child4, NULL, routine4, NULL);

    pthread_join(child, NULL);
    pthread_join(child2, NULL);
    pthread_join(child3, NULL);
    pthread_join(child4, NULL);

    printf("X is %d\n", *ptr);
    teardown();
}

