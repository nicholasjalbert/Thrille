/**
 * librace - Serialized Pthreads race detection library 
 * does *not* use thread specific storage
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */

#ifndef _LIBRACER_H_
#define _LIBRACER_H_

#include "../thrille-core/libpth.h"
#include "vctrack.h"
#include "locktrack.h"
#include "hybridtrack.h"

class RacerHandler: public Handler {
    protected: 
        HybridRaceTracker * eb;     
        map<thrID, bool> finish_map;
        ThreadTracker * threadtrack;

    public:
        RacerHandler();
        virtual ~RacerHandler(); 

    protected:	

        virtual void initializeThreadData(thrID);

        virtual void ThreadFinish(void * (*) (void *), void * status);
        virtual bool BeforeCreate(void *,
                pthread_t* newthread, 
                const pthread_attr_t *attr,
                void *(*start_routine) (void *), 
                void *arg, 
                thrID&); 
        virtual void AfterJoin(void *,
                int rev_val, 
                pthread_t th, 
                void** thread_return);
        virtual bool BeforeMutexLock(void *, pthread_mutex_t *mutex);
        virtual void AfterMutexUnlock(void *, int ret_val,
                pthread_mutex_t *mutex);
        virtual void AfterCondTimedwait(void *,
                int ret_val,
                pthread_cond_t * cond,
                pthread_mutex_t * lock,
                const struct timespec * time);
        virtual void AfterCondWait(void *,
                int ret_val,
                pthread_cond_t *cond, 
                pthread_mutex_t *mutex); 
        virtual bool BeforeCondSignal(void *, pthread_cond_t *cond); 

        virtual bool BeforeCondBroadcast(void *, pthread_cond_t * cond);
        virtual void myMemoryRead(void * iid, void * addr);
        virtual void myMemoryWrite(void * iid, void * addr);
        virtual bool BeforeExit(void *, void * value);
        
        friend void myMemoryRead(int, void*);
        friend void myMemoryWrite(int, void*);
};

#endif
