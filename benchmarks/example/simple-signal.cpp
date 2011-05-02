//signal

#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

int x;
bool flag;
int race_int;
pthread_t child, child2, child3, child4;
pthread_mutex_t mutex;
pthread_cond_t cond;

void setup() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

void teardown() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void * routine1(void * arg) {
    pthread_mutex_lock(&mutex);
    flag = true;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_lock(&mutex);
    x = 2;
    pthread_mutex_unlock(&mutex);
    
}


void * routine2(void * arg) {
    pthread_mutex_lock(&mutex);
    if (! flag) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    pthread_mutex_lock(&mutex);
    x = 4;
    pthread_mutex_unlock(&mutex);
}

void * routine3(void * arg) {
    race_int = 8;
}

void * routine4(void * arg) {
    race_int = 15;
}



int main(int argc, char *argv[]) {
    setup();
    x = 0;
    race_int = 0;
    flag = false;
    pthread_create(&child, NULL, routine1, NULL);
    pthread_create(&child2, NULL, routine2, NULL);
    pthread_create(&child3, NULL, routine3, NULL);
    pthread_create(&child4, NULL, routine4, NULL);

    pthread_join(child, NULL);
    pthread_join(child2, NULL);
    pthread_join(child3, NULL);
    pthread_join(child4, NULL);

    printf("X is %d\n", x);
    printf("race_int is %d\n", race_int);
    teardown();
}

