/**
 * libpth - Pthreads hooking library
 * 
 * Author - Chang-Seo Park (parkcs@cs.berkeley.edu)
 *          Nick Jalbert (jalbert@cs.berkeley.edu)
 *
 * <Legal matter>
 * */

#ifndef _LIBPTH_H
#define _LIBPTH_H

#include "coretypes.h"
#include "originals.h"
#include "threadtracker.h"

using namespace std;

/* register library constructors and destructors */
void pth_start(void) __attribute__((constructor));
void pth_end(void) __attribute__((destructor));

extern "C" {
    unsigned int sleep(unsigned int param0); 
    int usleep(useconds_t);
    int sigwait(const sigset_t *, int *);
}

class Handler {
    public:
        Handler() : single_threaded(true), safe_mode(false) {
            printf("Thrille:Starting Default thriller...\n");
            Originals::pthread_mutex_init(&_globalLock, NULL);
            threadtracker = new ThreadTracker();
            thrID myself = threadtracker->getNextTID();
            threadtracker->setTID(myself, Originals::pthread_self());
            Originals::pthread_mutex_init(&newthread_lock, NULL);
            Originals::pthread_cond_init(&newthread_cond, NULL);
        }

        virtual ~Handler() {
            Originals::pthread_mutex_destroy(&_globalLock);
            Originals::pthread_mutex_destroy(&newthread_lock);
            Originals::pthread_cond_destroy(&newthread_cond);
            delete threadtracker;
            printf("Thrille:Ending Default thriller...\n");
        }

    protected:
        map<thrID, bool> thread_start_map;
        ThreadTracker * threadtracker;
        pthread_mutex_t _globalLock;	
        pthread_mutex_t newthread_lock;
        pthread_cond_t newthread_cond;
        volatile bool single_threaded;
        volatile bool safe_mode;
        static volatile bool pth_is_initialized;
        
        void initializationDone();

        inline void lock() {
            Originals::pthread_mutex_lock(&_globalLock);
        }

        inline void unlock() {
            Originals::pthread_mutex_unlock(&_globalLock);
        }

        virtual thrID translateHandleToTID(pthread_t);
        virtual thrID getThrID();
        virtual void setTID(thrID, pthread_t);

        virtual void AfterInitialize() { }
        virtual void ThreadStart(void * (*) (void *), void *) {}
        virtual void ThreadFinish(void * (*) (void *), void *) {}

        PYTHON_PROTECTED_HANDLES

    private:

        PYTHON_PRIVATE_IMPL

        friend void pth_start(void);
        friend void * my_start_routine(void * my_arg);

        PYTHON_FRIEND_DECS

};

extern Handler* create_handler();

extern Handler * volatile pHandler;


#endif
