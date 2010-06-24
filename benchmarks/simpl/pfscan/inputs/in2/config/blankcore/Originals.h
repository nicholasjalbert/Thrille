#ifndef _ORIGINALS_H_INCLUDED
#define _ORIGINALS_H_INCLUDED

#include "mypthread.h"
#include <sys/types.h>
#include <semaphore.h>
#include <sched.h>

class Originals
{
    public:

        static void initialize();

        static volatile bool is_initialized();


/*
        static int usleep(useconds_t);
        static unsigned int sleep(unsigned int);
        static int sem_init(sem_t *, int, unsigned int);
        static int sem_destroy(sem_t *);
        static int sem_getvalue(sem_t *, int *);
        static int sem_post(sem_t *);
        static int sem_wait(sem_t *);
        static int sem_trywait(sem_t *);
        static int sem_timedwait(sem_t *, const struct timespec *);
        static int sched_yield(void);
        */
        
        PYTHON_STATIC_PUBLIC 

    private:

        static void _initialize();

        static volatile bool _initialized;

        static pthread_once_t _once_control;
/*
        static int (* volatile _usleep) (useconds_t);
        static unsigned int (* volatile _sleep) (unsigned int);
        static int (* volatile _sem_init) (sem_t *, int, unsigned int);
        static int (* volatile _sem_destroy) (sem_t *);
        static int (* volatile _sem_getvalue) (sem_t *, int *);
        static int (* volatile _sem_post) (sem_t *);
        static int (* volatile _sem_wait) (sem_t *);
        static int (* volatile _sem_trywait) (sem_t *);
        static int (* volatile _sem_timedwait) (sem_t *, const struct timespec *);
        static int (* volatile _sched_yield) (void);
        */

        PYTHON_STATIC_PRIVATE 

};

#endif 
