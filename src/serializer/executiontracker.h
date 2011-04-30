/**
 * executiontracker - tracks the status of a program's execution
 *                    while serializing it
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _EXECTRACKER_H_
#define _EXECTRACKER_H_

#include "serializertypes.h"
#include "logger.h"
#include "barriertracker.h"

class ExecutionTracker {
    private:
        map<thrID, sem_t *> sem_map; 
        map<thrID, bool> finish_map;
        map<thrID, bool> exit_map;
        map<thrID, thrID> join_map;
        map<thrID, pthread_mutex_t *> lock_ready_map;
        map<thrID, pthread_mutex_t *> lock_wait_map;
        map<thrID, sem_t *> sem_ready_map;
        map<thrID, sem_t *> sem_wait_map;

        thrID main_thread_id;


        pthread_cond_t * signal_cond;


    protected:
        pthread_mutex_t * global_lock;
        vector<pthread_mutex_t *> mutex_destroy;
        map<thrID, sem_t *> wait_sem_map;
        map<thrID, bool> wait_ready_map;
        vector<pthread_cond_t *> cond_destroy;
        vector<sem_t *> sem_destroy;
        vector<pthread_barrier_t *> barrier_destroy;
        int preemptions;
        int nonpreemptions;
        int contextswitch;
        int skipped_comparisons;
        Logger * log;
        BarrierTracker * bt;
        map<thrID, bool> was_signalled_map;
        int lock_count;
        map<thrID, bool> enable_map;
        map<thrID, pthread_cond_t *> cond_map;

        virtual void thrilleCheckpoint(thrID, void *);
        virtual void thrilleSchedPoint(thrID, void *);

        virtual bool BeforeMutexattrSettype(thrID,
                void * ret_addr,
                pthread_mutexattr_t *,
                int);
        
        virtual void followingChoiceFromLog(SchedPointInfo *); 
        virtual bool BeforeMutexTrylock(thrID, void *, pthread_mutex_t *); 
        virtual int SimulateMutexTrylock(thrID, void *, pthread_mutex_t *); 
        virtual void AfterMutexTrylock(thrID, void *, int, pthread_mutex_t *); 
        virtual bool BeforeGetschedparam(thrID, void *, 
                pthread_t, int *, struct sched_param *); 
        virtual bool BeforeKeyCreate(thrID, void *,
                pthread_key_t *, void (*)(void*));
        virtual bool BeforeSetspecific(thrID, void *,
                pthread_key_t, const void *);
        virtual bool BeforeGetspecific(thrID, void *, pthread_key_t);
        virtual bool BeforeAttrSetstacksize(thrID, void *,
                pthread_attr_t *, size_t);
        virtual bool BeforeAttrSetdetachstate(thrID, void *,
                pthread_attr_t *, int);
        virtual bool BeforeAttrGetschedparam(thrID, void *, 
                const pthread_attr_t *, struct sched_param *);
        virtual bool BeforeAttrSetschedparam(thrID, void *,
                pthread_attr_t *, const struct sched_param *);
        virtual bool BeforeEqual(thrID, void *, pthread_t, pthread_t);
        virtual bool BeforeAttrDestroy(thrID, void *, pthread_attr_t *);

        void initializeThreadData(thrID);
        virtual void finalizeThread(thrID, void *);
        virtual void ThreadStart(thrID, void * (*) (void *), void *); 
        virtual void ThreadFinish(thrID, void * (*) (void *), void *); 
        virtual thrID schedule(SchedPointInfo *);
        virtual bool BeforeMutexDestroy(thrID, void *, pthread_mutex_t *);
        virtual int SimulateMutexDestroy(thrID, void *, pthread_mutex_t *);
        virtual bool BeforeCondDestroy(thrID, void *, pthread_cond_t *);
        virtual int SimulateCondDestroy(thrID, void *, pthread_cond_t *);
        virtual bool BeforeBarrierDestroy(thrID, void *, pthread_barrier_t *);
        virtual int SimulateBarrierDestroy(thrID, void *, pthread_barrier_t *);
        void checkForDanglingThreads();


        bool BeforeSetcanceltype(thrID myself, void * retaddr, int, int *);
        bool isSchedulingPoint(void *);
        bool tryToSchedule();
        virtual void recordScheduleInfo(thrID, thrID, string, void *);

        virtual void thrilleAssert(thrID, void *, bool);
        virtual void semaphoreIsPosted(thrID, sem_t *);
        virtual void threadHasPassedSemaphore(thrID, sem_t *);
        virtual void addThreadReadyOnSemaphore(thrID, sem_t *);
        virtual void addThreadWaitingOnSemaphore(thrID, sem_t *);
        virtual void determineSemaphoreStatus(thrID, sem_t *);
        virtual int checkSemaphoreValue(sem_t *);
        virtual bool BeforeSemWait(thrID, void *, sem_t *);
        virtual int SimulateSemWait(thrID, void *, sem_t *);
        virtual bool BeforeSemPost(thrID, void *, sem_t *);
        virtual int SimulateSemPost(thrID, void *, sem_t *);
        virtual bool BeforeSemInit(thrID, void *, sem_t *, int, unsigned int);
        virtual bool BeforeBarrierInit(thrID, void *, pthread_barrier_t *, 
                const pthread_barrierattr_t *, unsigned);
        virtual bool BeforeBarrierWait(thrID, void *, pthread_barrier_t *);

        virtual bool BeforeOnce(thrID,
                void *, pthread_once_t *, void (*) (void));
        virtual bool BeforeMutexInit(thrID, void *, pthread_mutex_t *,
                const pthread_mutexattr_t *);
        virtual bool BeforeCondInit(thrID, void *, pthread_cond_t *,
                const pthread_condattr_t *);
        virtual bool BeforeSelf(thrID, void *);
        virtual bool BeforeSemDestroy(thrID, void *, sem_t *);
        virtual int SimulateSemDestroy(thrID, void *, sem_t *);
        virtual bool BeforeAttrInit(thrID, void *, pthread_attr_t *);
        virtual bool BeforeAttrSetscope(thrID, void *, pthread_attr_t *, int);

        virtual bool BeforeCreate(thrID,
                thrID,
                void *,
                pthread_t *, 
                const pthread_attr_t *,
                void *(*)(void *),
                void *);
        virtual void AfterCreate(thrID,
                thrID,
                void *,
                int,
                pthread_t *,
                const pthread_attr_t *,
                void *(*)(void *),
                void * );
        virtual bool BeforeJoin(thrID, thrID, void *, pthread_t, void**);
        virtual bool BeforeMutexLock(thrID, void *,
                pthread_mutex_t *, bool);
        virtual void checkCondValidity(pthread_cond_t *);
        virtual void condIsValid(pthread_cond_t *);
        virtual void checkSemValidity(sem_t *);
        virtual void semIsValid(sem_t *);
        virtual void lockIsValid(pthread_mutex_t *);
        virtual void checkBarrierValidity(pthread_barrier_t *);
        virtual void barrierIsValid(pthread_barrier_t *);
        virtual int SimulateMutexLock(thrID, void *, pthread_mutex_t *, bool);
        virtual bool BeforeMutexUnlock(thrID, void *, pthread_mutex_t *, bool);
        virtual void AfterMutexUnlock(thrID, void *,
                int, pthread_mutex_t *, bool);
        virtual bool BeforeCondWait(thrID,
                void *, 
                pthread_cond_t *, 
                pthread_mutex_t *);
        virtual int SimulateCondWait(thrID,
                void *,
                pthread_cond_t *, 
                pthread_mutex_t *); 
                
        virtual bool BeforeCondSignal(thrID, void *, pthread_cond_t *);
        virtual int SimulateCondSignal(thrID, void *, pthread_cond_t *);
        virtual bool BeforeCondBroadcast(thrID, void *, pthread_cond_t *); 
        virtual int SimulateCondBroadcast(thrID, void *, pthread_cond_t *);
        virtual void myMemoryRead(thrID, void *, void *);
        virtual void myMemoryWrite(thrID, void *, void *);
    
        virtual void disableThread(thrID);
        virtual void enableThread(thrID);
        virtual bool isEnabled(thrID);
        virtual bool isDisabled(thrID);
        virtual void pauseThread(thrID);
        virtual void waitPauseThread(thrID);
        virtual void wakeThread(thrID);
        virtual void waitWakeThread(thrID);
        virtual void registerThreadJoin(thrID, thrID);
        virtual bool threadIsFinished(thrID);
        virtual void enableJoiningThread(thrID);
        virtual void addThreadWaitingOnLock(thrID, pthread_mutex_t *);
        virtual void addThreadReadyOnLock(thrID, pthread_mutex_t *);
        virtual void threadHasAcquiredLock(thrID, pthread_mutex_t *);
        virtual void threadHasReleasedLock(thrID, pthread_mutex_t *);
        virtual void determineLockStatus(thrID, pthread_mutex_t *);
        virtual void wakeThreadWaitingOnCond(SignalPointInfo *);
        virtual void signalThreadReady(thrID);
        virtual void waitThreadReady(thrID, thrID);
        virtual bool isSignalMissed(SignalPointInfo *);
        virtual thrID selectThreadWaitingOnCond(SignalPointInfo *);
        
        virtual bool BeforeMutexattrInit(thrID, void*, pthread_mutexattr_t*);
        
            
            virtual void scheduleThreadWaitingOnCond(thrID, SignalPointInfo *);
        virtual thrID pickNextSchedulingChoice(SchedPointInfo *);
        virtual bool BeforeCondTimedwait(thrID, 
                void *,
                pthread_cond_t *, 
                pthread_mutex_t *,
                const struct timespec *);
        virtual int SimulateCondTimedwait(thrID,
                void *,
                pthread_cond_t *, 
                pthread_mutex_t *,
                const struct timespec *);
        virtual void launchThread(thrID target, SchedPointInfo *);
        virtual bool BeforeExit(thrID, void *, void *);
        virtual void SimulateExit(thrID, void *, void *);
        virtual bool BeforeSleep(thrID, void *, unsigned int);
        virtual unsigned int SimulateSleep(thrID, void *, unsigned int);
        virtual bool BeforeUsleep(thrID, void *, useconds_t);
        virtual int SimulateUsleep(thrID, void *, useconds_t);
       
        virtual void glock();
        virtual void gunlock();
        
        virtual void handleAfterCreate(thrID, void *);
        virtual void handleBeforeJoin(thrID, thrID, void *); 
        virtual void handleBeforeMutexLock(thrID, void *, pthread_mutex_t *,
                bool);
        virtual void handleAfterMutexUnlock(thrID, void *,
                pthread_mutex_t *, bool);
        virtual void handleSimulateCondWait(thrID,
                void *,
                pthread_cond_t *, 
                pthread_mutex_t *);
        virtual void handleSimulateCondTimedwait(thrID, 
                void *,
                pthread_cond_t *, 
                pthread_mutex_t *);
        virtual void handleSimulateCondSignal(thrID,
               void *, pthread_cond_t *);
        virtual void handleSimulateCondBroadcast(thrID,
               void *, pthread_cond_t *); 
        virtual void handleMyMemoryRead(thrID, void *, void *);
        virtual void handleMyMemoryWrite(thrID, void *, void *);
        virtual void handleThreadStart(thrID, void * (*) (void *));
        virtual void handleThreadFinish(thrID);
        virtual void handleMyUsleep(thrID, void *);
        virtual void handleMySleep(thrID, void *);
        virtual bool BeforeSigwait(thrID, void *, const sigset_t *, int *);

        virtual void segfault(thrID);

        virtual void deadlockCheck();

        virtual void compareScheduleSynchronization(string,string); 

    public:

        void constructorHelper();
        void destructorHelper();
        ExecutionTracker(thrID);
        ExecutionTracker();
        virtual ~ExecutionTracker();

        friend class SerializerHandler;
        friend void catch_sigseg(int);
        friend class ExecutionTrackerTestSuite;
        friend void * threadSimulateCondWait(void *);
        friend void * threadSignalThreadReadyA(void * arg); 
        friend void * threadSignalThreadReadyB(void * arg);
        friend void * threadWakeThreadWaitingOnCond(void * arg);
        friend void myMemoryWrite(int, void *);
        friend void myMemoryRead(int, void *);

};


#endif

