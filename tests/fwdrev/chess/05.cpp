#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

int x;
int * ptr;
pthread_t child, child2;
pthread_mutex_t mutex;

void * routine1(void * arg) {
    pthread_mutex_lock(&mutex);
    ptr = NULL;
    pthread_mutex_unlock(&mutex);
}

void * routine2(void * arg) {
    pthread_mutex_lock(&mutex);
    ptr = &x;
    pthread_mutex_unlock(&mutex);
}


int main(int argc, char *argv[]) {
    pthread_mutex_init(&mutex, NULL);
    x = 0;
    ptr = &x;
    pthread_create(&child, NULL, routine1, NULL);
    pthread_create(&child2, NULL, routine2, NULL);
    int me = (*ptr);
    pthread_join(child, NULL);
    pthread_join(child2, NULL);
    pthread_mutex_destroy(&mutex);
}

