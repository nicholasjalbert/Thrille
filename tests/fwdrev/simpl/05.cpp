#include <errno.h>
#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

int x;
int * y;
bool flag;
pthread_t child, child2;
pthread_mutex_t mutex;

void setup() {
    pthread_mutex_init(&mutex, NULL);
    x = 0;
    flag = false;
}

void teardown() {
    pthread_mutex_destroy(&mutex);
}

void * routine1(void * arg) {
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    flag = true;
    y = NULL;

}

void * routine2(void * arg) {
    while (! flag) {
        usleep(30);
    }
    y = &x;
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[]) {
    setup();
    
    pthread_create(&child, NULL, routine1, NULL);
    pthread_create(&child2, NULL, routine2, NULL);
    pthread_join(child, NULL);
    pthread_join(child2, NULL);
    pthread_mutex_lock(&mutex);
    y = NULL;
    int z = *y;
    pthread_mutex_unlock(&mutex);
    
    teardown();
}

