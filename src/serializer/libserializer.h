/**
* libserializer - serializes a multithreaded execution and tracks enabled
*                 threads
* 
* Author -  Nick Jalbert (jalbert@cs.berkeley.edu)
*
* <Legal matter>
*/

#ifndef _LIBSERIALIZER_H_
#define _LIBSERIALIZER_H_

#include "serializertypes.h"
#include "executiontracker.h"
#include "racer.h"

extern "C" {
    void thrilleAssertC(int);
    void * thrilleMallocC(size_t);
    void thrilleCheckpointC();
    void thrilleSchedPointC();
}

class SerializerHandler : public Handler {
    private: 
        ExecutionTracker * controller;
        Racer * race;
        pthread_mutex_t * race_detection_lock;

    public:
        SerializerHandler();
        virtual ~SerializerHandler(); 

    protected:
        virtual void AfterInitialize();
        virtual void ThreadStart(void * (*) (void *), void *); 
        virtual void ThreadFinish(void * (*) (void *), void *); 

        bool isSchedulingPoint(void *);
        bool tryToSchedule();

        void thrilleCheckpoint(void *);

        virtual bool BeforeGetschedparam(void *, pthread_t, int *,
                struct sched_param *);
        virtual bool BeforeKeyCreate(void *, pthread_key_t *, void (*)(void*));
        virtual bool BeforeSetspecific(void *, pthread_key_t, const void *);
        virtual bool BeforeGetspecific(void *, pthread_key_t);
        virtual bool BeforeMutexTrylock(void *, pthread_mutex_t *); 
        virtual int SimulateMutexTrylock(void *, pthread_mutex_t *); 
        virtual void AfterMutexTrylock(void *, int, pthread_mutex_t *); 
        virtual bool BeforeAttrDestroy(void * ret_addr, pthread_attr_t *);
       
        virtual bool BeforeEqual(void *, pthread_t, pthread_t);
        virtual bool BeforeAttrSetschedparam(void *,
                pthread_attr_t *,
                const struct sched_param *);
        virtual bool BeforeAttrSetstacksize(void *, pthread_attr_t *, size_t);
        virtual bool BeforeAttrSetdetachstate(void * ret_addr,
                pthread_attr_t *, int); 
        virtual bool BeforeAttrGetschedparam(void *, const pthread_attr_t *, 
                struct sched_param *);

        virtual bool BeforeMutexattrSettype(void * ret_addr,
                pthread_mutexattr_t *,
                int);

        virtual bool BeforeOnce(void *, pthread_once_t *, void (*)(void));
        virtual bool BeforeMutexInit(void *, pthread_mutex_t *,
                const pthread_mutexattr_t *);
        virtual bool BeforeCondInit(void *, pthread_cond_t *,
                const pthread_condattr_t *);
        virtual bool BeforeMutexattrInit(void *, pthread_mutexattr_t *);
        virtual bool BeforeBarrierDestroy(void *, pthread_barrier_t *);
        virtual int SimulateBarrierDestroy(void *, pthread_barrier_t *);
        virtual bool BeforeSelf(void *);
        virtual bool BeforeSemDestroy(void *, sem_t *);
        virtual int SimulateSemDestroy(void *, sem_t *);
        virtual bool BeforeAttrInit(void *, pthread_attr_t *);
        virtual bool BeforeAttrSetscope(void *, pthread_attr_t *, int); 

        bool BeforeSetcanceltype(void * ret_addr, int, int*);
        virtual bool BeforeCreate(void *,
                pthread_t *, 
                const pthread_attr_t *,
                void *(*)(void *),
                void *,
                thrID &); 
        virtual void AfterCreate(void *,
                int,
                pthread_t *,
                const pthread_attr_t *,
                void *(*)(void *),
                void *,
                thrID &);

        virtual bool BeforeBarrierInit(void *, pthread_barrier_t *,
                const pthread_barrierattr_t *, unsigned);
        virtual bool BeforeBarrierWait(void *, pthread_barrier_t *);

        virtual bool BeforeSemWait(void *, sem_t *);
        virtual int SimulateSemWait(void *, sem_t *);
        virtual bool BeforeSemPost(void *, sem_t *);
        virtual int SimulateSemPost(void *, sem_t *);
        virtual bool BeforeSemInit(void *, sem_t *, int, unsigned int);
        
        virtual bool BeforeJoin(void *, pthread_t, void**);
        virtual bool BeforeMutexLock(void *, pthread_mutex_t *);
        virtual int SimulateMutexLock(void *, pthread_mutex_t *);
        virtual void AfterMutexLock(void *, int, pthread_mutex_t *);
        virtual bool BeforeMutexDestroy(void *, pthread_mutex_t *);
        virtual int SimulateMutexDestroy(void *, pthread_mutex_t *);
        virtual bool BeforeCondDestroy(void *, pthread_cond_t *);
        virtual int SimulateCondDestroy(void *, pthread_cond_t *);
        virtual bool BeforeMutexUnlock(void *, pthread_mutex_t *);
        virtual void AfterMutexUnlock(void *, int, pthread_mutex_t *);
        virtual bool BeforeCondWait(void *, pthread_cond_t *, 
                pthread_mutex_t *);
        virtual int SimulateCondWait(void *, pthread_cond_t *, 
                pthread_mutex_t *);
        virtual void AfterCondWait(void *, int,
                pthread_cond_t *, pthread_mutex_t *);
        virtual bool BeforeCondTimedwait(void *, pthread_cond_t *, 
                pthread_mutex_t *, 
                const struct timespec *);
        virtual int SimulateCondTimedwait(void *, pthread_cond_t *, 
                pthread_mutex_t *,
                const struct timespec *);
        virtual void AfterCondTimedwait(void *, 
                int, 
                pthread_cond_t *,
                pthread_mutex_t *,
                const struct timespec *);
        virtual bool BeforeCondSignal(void *, pthread_cond_t *);
        virtual int SimulateCondSignal(void *, pthread_cond_t *);
        virtual bool BeforeCondBroadcast(void *, pthread_cond_t *); 
        virtual int SimulateCondBroadcast(void *, pthread_cond_t *);
        virtual void myMemoryRead(void * , void *);
        virtual void myMemoryWrite(void *, void *);
        virtual bool BeforeUsleep(void *, useconds_t);
        virtual int SimulateUsleep(void *, useconds_t);
        virtual bool BeforeSleep(void *, unsigned int);
        virtual unsigned int SimulateSleep(void *, unsigned int);
        virtual bool BeforeExit(void *, void *);
        virtual void SimulateExit(void *, void *);
        virtual void AfterJoin(void *, int, pthread_t, void **);

        virtual bool BeforeSigwait(void *, const sigset_t *, int *);

        virtual ExecutionTracker * getNewExecutionTracker(thrID);
        virtual Racer * getNewRaceDetect(thrID);
        virtual void thrilleAssert(void *, bool);
        virtual void thrilleSchedPoint(void *);
        virtual void thrilleSegfault();

        friend void myMemoryRead(int, void*);
        friend void myMemoryWrite(int, void*);
        friend void catch_sigseg(int);
        friend void thrilleSchedPointC();
        friend void thrilleSchedPointCPP();
        friend void thrilleAssertC(int);
        friend void thrilleAssertCPP(bool);
        friend void thrilleCheckpointC(void);
        friend void thrilleCheckpointCPP(void);
        friend class LibSerializerTestSuite;

        

};


#endif
