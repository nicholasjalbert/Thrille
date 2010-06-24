#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

int x;
bool flag;
int * ptr;
pthread_t child, child2, child3;
pthread_mutex_t mutex;
pthread_cond_t cond;

void * routine1(void * arg) {
    pthread_mutex_lock(&mutex);
    while (!flag) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void * routine2(void * arg) {
    pthread_mutex_lock(&mutex);
    flag = true;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}


int main(int argc, char *argv[]) {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&child2, NULL, routine1, NULL);
    pthread_create(&child3, NULL, routine2, NULL);


    pthread_join(child2, NULL);
    pthread_join(child3, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

