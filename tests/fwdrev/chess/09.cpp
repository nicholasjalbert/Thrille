//Check that we're handling signals correctly (i.e. if we have
// more than one choice, we explore signal to each thread)
//
// if we are handling them correctly, a segfault will be found
// with preemption bound 0, otherwise it will not be.

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

int x;
bool flag, flag_child0, flag_child1;
int * ptr;
pthread_t child0, child1, child2, child3;
pthread_mutex_t mutex, mutex_child0, mutex_child1;
pthread_cond_t cond, cond_child0, cond_child1;

void * routine0(void * arg) {
    pthread_mutex_lock(&mutex);

    pthread_mutex_lock(&mutex_child0);
    flag_child0 = true;
    pthread_cond_signal(&cond_child0);
    pthread_mutex_unlock(&mutex_child0);

    while (!flag) {
        pthread_cond_wait(&cond, &mutex);
        ptr = &x;
    }
    pthread_mutex_unlock(&mutex);

}

void * routine1(void * arg) {
    pthread_mutex_lock(&mutex);
    
    pthread_mutex_lock(&mutex_child1);
    flag_child1 = true;
    pthread_cond_signal(&cond_child1);
    pthread_mutex_unlock(&mutex_child1);

    while (!flag) {
        pthread_cond_wait(&cond, &mutex);
        ptr = NULL;
    }
    pthread_mutex_unlock(&mutex);
}

void * routine2(void * arg) {
    pthread_mutex_lock(&mutex);
    flag = true;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void * routine3(void * arg) {
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
}


int main(int argc, char *argv[]) {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex_child0, NULL);
    pthread_cond_init(&cond_child0, NULL);
    pthread_mutex_init(&mutex_child1, NULL);
    pthread_cond_init(&cond_child1, NULL);



    flag = false;
    flag_child0 = false;
    flag_child1 = false;
    x = 0;
    ptr = &x;

    /* create first waiter */
    pthread_create(&child0, NULL, routine0, NULL);
    
    /* force thread 1 to be first waiter */
    pthread_mutex_lock(&mutex_child0);
    while (!flag_child0) {
        pthread_cond_wait(&cond_child0, &mutex_child0);
    }
    pthread_mutex_unlock(&mutex_child0);

    /* create second waiter */ 
    pthread_create(&child1, NULL, routine1, NULL);


    /* force thread 2 to be second waiter */
    pthread_mutex_lock(&mutex_child1);
    while (!flag_child1) {
        pthread_cond_wait(&cond_child1, &mutex_child1);
    }
    pthread_mutex_unlock(&mutex_child1);

    pthread_create(&child2, NULL, routine2, NULL);
    pthread_create(&child3, NULL, routine3, NULL);


    pthread_join(child2, NULL);
    pthread_join(child3, NULL);

    printf("Result: %d\n", (*ptr));
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

