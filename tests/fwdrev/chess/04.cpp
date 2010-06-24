#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

int * ptr;
pthread_t child;
pthread_mutex_t mutex;

void * routine1(void * arg) {
    pthread_mutex_lock(&mutex);
    ptr = NULL;
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[]) {
    pthread_mutex_init(&mutex, NULL);
    pthread_create(&child, NULL, routine1, NULL);
    int me = (*ptr);
    pthread_join(child, NULL);
    pthread_mutex_destroy(&mutex);
}

