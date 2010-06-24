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

#include "Originals.h"
#include "mypthread.h"
#include <cstdlib>
#include <cstdio>
#include <pthread.h>
#include <dlfcn.h>
#include <stdarg.h>


#ifndef UNLOCKASSERT
#define safe_assert(cond) \
    if (!(cond))  { \
        printf("\nThrille: safe assert fail: safe_assert(%s):", #cond); \
        printf(" \n\tfunction: %s\n\tfile: %s\n\tline: %d\n", __FUNCTION__, __FILE__, __LINE__); \
        fflush(stdout); \
        _Exit(3); \
    }
#else
#define safe_assert(cond) \
    if (!(cond))  { \
        printf("\nExecTracker: safe assert fail: safe_assert(%s):", #cond); \
        printf(" \n\tfunction: %s\n\tfile: %s\n\tline: %d\n", __FUNCTION__, __FILE__, __LINE__); \
        fflush(stdout); \
        printf("global lock released\n"); \
        log->assertFail(__FILE__, __LINE__); \
        destructorHelper(); \
        Originals::pthread_mutex_trylock(global_lock);  \
        Originals::pthread_mutex_unlock(global_lock); \
        _Exit(3); \
    }


#endif


// Library initialization
void pth_start(void) __attribute__((constructor));
void pth_end(void) __attribute__((destructor));

class ThreadInfo;

extern "C" {
    unsigned int sleep(unsigned int param0); 
    int usleep(useconds_t);
    int sigwait(const sigset_t *, int *);
}

// "null" pthread handler 
class Handler {
    public:
        Handler() : _single_thread(true), safe_mode(false) {
            dbgPrint("Starting Default thriller...\n");
            Originals::pthread_mutex_init(&_globalLock, NULL);
        }

        virtual ~Handler() {
            Originals::pthread_mutex_destroy(&_globalLock);
            dbgPrint("Ending Default thriller...\n");
        }

        int dbgPrint(const char* format, ...) {
            va_list args;
            fprintf(stderr, "Thrille:");
            va_start(args, format);
            vfprintf(stderr, format, args);
            va_end(args);
            return 0;
        }

    protected:
        pthread_mutex_t _globalLock;	
        volatile bool _single_thread;
        bool safe_mode;

        inline void lock() {
            ++insideInst;
            Originals::pthread_mutex_lock(&_globalLock);
            --insideInst;
        }

        inline void unlock() {
            ++insideInst;
            Originals::pthread_mutex_unlock(&_globalLock);
            --insideInst;
        }

        virtual void AfterInitialize() { }
        virtual void ThreadStart(void * (*) (void *), void *) {}
        virtual void ThreadFinish(void * (*) (void *), void *) {}

        PYTHON_PROTECTED_HANDLES

    private:
        static const char* UNKNOWN;
        static __thread int insideInst;

        PYTHON_PRIVATE_IMPL

        friend class Initializer;
        friend void * my_start_routine(void * my_arg);

        PYTHON_FRIEND_DECS

};

extern Handler* create_handler();

extern Handler * volatile pHandler;


#endif
