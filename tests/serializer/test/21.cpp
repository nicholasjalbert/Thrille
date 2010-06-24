//test if debug info is working

#include <pthread.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include <execinfo.h>

using namespace std;

int x;
bool y;
sem_t sem;
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

void * routine(void * arg) {
}

void bt() {

    void * ret = __builtin_return_address(0);
    ostringstream * str = new ostringstream();
    (*str) << "addr2line -e 21 " << ret;
    printf("%s\n", str->str().c_str());
    system(str->str().c_str());
}

int main(int argc, char *argv[]) {
    setup();
    bt();
    teardown();
}

