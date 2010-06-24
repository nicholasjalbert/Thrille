#ifndef _CORETYPES_H
#define _CORETYPES_H

#include <cstdlib>
#include <cstdio>
#include <dlfcn.h>
#include <stdarg.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <string>

#include <vector>
#include <map>
#include <set>

#define UNRECOVERABLE_ERROR 5

typedef int thrID;

#ifndef UNLOCKASSERT
#define safe_assert(cond) \
    if (!(cond))  { \
        printf("\nThrille: safe assert fail: safe_assert(%s):", #cond); \
        printf(" \n\tfunction: %s\n\tfile: %s\n\tline: %d\n", __FUNCTION__, __FILE__, __LINE__); \
        fflush(stdout); \
        _Exit(UNRECOVERABLE_ERROR); \
    }
#else
#define safe_assert(cond) \
    if (!(cond))  { \
        printf("\nExecTracker: safe assert fail: safe_assert(%s):", #cond); \
        printf(" \n\tfunction: %s\n\tfile: %s\n\tline: %d\n", __FUNCTION__, __FILE__, __LINE__); \
        fflush(stdout); \
        log->assertFail(__FILE__, #cond,  __LINE__); \
        destructorHelper(); \
        Originals::pthread_mutex_trylock(global_lock);  \
        Originals::pthread_mutex_unlock(global_lock); \
        _Exit(UNRECOVERABLE_ERROR); \
    }


#endif



#endif
