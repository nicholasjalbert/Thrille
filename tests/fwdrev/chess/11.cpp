#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>


int x;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * f1(void * arg) {
    pthread_mutex_lock(&mutex);
    while (x == 0) {
        pthread_mutex_unlock(&mutex);
        usleep(1);
        pthread_mutex_lock(&mutex);
    }
  pthread_mutex_unlock(&mutex);
  return NULL;
}

int main() {
  x = 0;
  pthread_t p1, p2;
  pthread_create(&p1,NULL,f1,NULL);
  pthread_mutex_lock(&mutex);
  x = 1;
  pthread_mutex_unlock(&mutex);
  pthread_join(p1,NULL);

  return 0;
}
