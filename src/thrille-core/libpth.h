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

        virtual bool BeforeSemWait(void * ret_addr, sem_t *);
        virtual int SimulateSemWait(void * ret_addr, sem_t *);
        virtual void AfterSemWait(void * ret_addr, int, sem_t *);

        virtual bool BeforeMutexLock(void * ret_addr, pthread_mutex_t *);
        virtual int SimulateMutexLock(void * ret_addr, pthread_mutex_t *);
        virtual void AfterMutexLock(void * ret_addr, int, pthread_mutex_t *);

        virtual bool BeforeSleep(void * ret_addr, unsigned int);
        virtual unsigned int SimulateSleep(void * ret_addr, unsigned int);
        virtual void AfterSleep(void * ret_addr, unsigned int, unsigned int);

        virtual bool BeforeCondattrDestroy(void * ret_addr, pthread_condattr_t *);
        virtual int SimulateCondattrDestroy(void * ret_addr, pthread_condattr_t *);
        virtual void AfterCondattrDestroy(void * ret_addr, int, pthread_condattr_t *);

        virtual bool BeforeMutexattrGetpshared(void * ret_addr, const pthread_mutexattr_t *, int *);
        virtual int SimulateMutexattrGetpshared(void * ret_addr, const pthread_mutexattr_t *, int *);
        virtual void AfterMutexattrGetpshared(void * ret_addr, int, const pthread_mutexattr_t *, int *);

        virtual bool BeforeSetconcurrency(void * ret_addr, int);
        virtual int SimulateSetconcurrency(void * ret_addr, int);
        virtual void AfterSetconcurrency(void * ret_addr, int, int);

        virtual bool BeforeGetconcurrency(void * ret_addr);
        virtual int SimulateGetconcurrency(void * ret_addr);
        virtual void AfterGetconcurrency(void * ret_addr, int);

        virtual bool BeforeSemDestroy(void * ret_addr, sem_t *);
        virtual int SimulateSemDestroy(void * ret_addr, sem_t *);
        virtual void AfterSemDestroy(void * ret_addr, int, sem_t *);

        virtual bool BeforeTestcancel(void * ret_addr);
        virtual void SimulateTestcancel(void * ret_addr);
        virtual void AfterTestcancel(void * ret_addr);

        virtual bool BeforeAttrGetscope(void * ret_addr, const pthread_attr_t *, int *);
        virtual int SimulateAttrGetscope(void * ret_addr, const pthread_attr_t *, int *);
        virtual void AfterAttrGetscope(void * ret_addr, int, const pthread_attr_t *, int *);

        virtual bool BeforeMutexInit(void * ret_addr, pthread_mutex_t *, const pthread_mutexattr_t *);
        virtual int SimulateMutexInit(void * ret_addr, pthread_mutex_t *, const pthread_mutexattr_t *);
        virtual void AfterMutexInit(void * ret_addr, int, pthread_mutex_t *, const pthread_mutexattr_t *);

        virtual bool BeforeSemInit(void * ret_addr, sem_t *, int, unsigned int);
        virtual int SimulateSemInit(void * ret_addr, sem_t *, int, unsigned int);
        virtual void AfterSemInit(void * ret_addr, int, sem_t *, int, unsigned int);

        virtual bool BeforeMutexTrylock(void * ret_addr, pthread_mutex_t *);
        virtual int SimulateMutexTrylock(void * ret_addr, pthread_mutex_t *);
        virtual void AfterMutexTrylock(void * ret_addr, int, pthread_mutex_t *);

        virtual bool BeforeRwlockattrInit(void * ret_addr, pthread_rwlockattr_t *);
        virtual int SimulateRwlockattrInit(void * ret_addr, pthread_rwlockattr_t *);
        virtual void AfterRwlockattrInit(void * ret_addr, int, pthread_rwlockattr_t *);

        virtual bool BeforeMutexattrSetpshared(void * ret_addr, pthread_mutexattr_t *, int);
        virtual int SimulateMutexattrSetpshared(void * ret_addr, pthread_mutexattr_t *, int);
        virtual void AfterMutexattrSetpshared(void * ret_addr, int, pthread_mutexattr_t *, int);

        virtual bool BeforeRwlockattrDestroy(void * ret_addr, pthread_rwlockattr_t *);
        virtual int SimulateRwlockattrDestroy(void * ret_addr, pthread_rwlockattr_t *);
        virtual void AfterRwlockattrDestroy(void * ret_addr, int, pthread_rwlockattr_t *);

        virtual bool BeforeBarrierDestroy(void * ret_addr, pthread_barrier_t *);
        virtual int SimulateBarrierDestroy(void * ret_addr, pthread_barrier_t *);
        virtual void AfterBarrierDestroy(void * ret_addr, int, pthread_barrier_t *);

        virtual bool BeforeGetschedparam(void * ret_addr, pthread_t, int *, struct sched_param *);
        virtual int SimulateGetschedparam(void * ret_addr, pthread_t, int *, struct sched_param *);
        virtual void AfterGetschedparam(void * ret_addr, int, pthread_t, int *, struct sched_param *);

        virtual bool BeforeAttrGetschedparam(void * ret_addr, const pthread_attr_t *, struct sched_param *);
        virtual int SimulateAttrGetschedparam(void * ret_addr, const pthread_attr_t *, struct sched_param *);
        virtual void AfterAttrGetschedparam(void * ret_addr, int, const pthread_attr_t *, struct sched_param *);

        virtual bool BeforeCancel(void * ret_addr, pthread_t);
        virtual int SimulateCancel(void * ret_addr, pthread_t);
        virtual void AfterCancel(void * ret_addr, int, pthread_t);

        virtual bool BeforeCondWait(void * ret_addr, pthread_cond_t *, pthread_mutex_t *);
        virtual int SimulateCondWait(void * ret_addr, pthread_cond_t *, pthread_mutex_t *);
        virtual void AfterCondWait(void * ret_addr, int, pthread_cond_t *, pthread_mutex_t *);

        virtual bool BeforeCondattrGetpshared(void * ret_addr, const pthread_condattr_t *, int *);
        virtual int SimulateCondattrGetpshared(void * ret_addr, const pthread_condattr_t *, int *);
        virtual void AfterCondattrGetpshared(void * ret_addr, int, const pthread_condattr_t *, int *);

        virtual bool BeforeSpinTrylock(void * ret_addr, pthread_spinlock_t *);
        virtual int SimulateSpinTrylock(void * ret_addr, pthread_spinlock_t *);
        virtual void AfterSpinTrylock(void * ret_addr, int, pthread_spinlock_t *);

        virtual bool BeforeAttrGetinheritsched(void * ret_addr, const pthread_attr_t *, int *);
        virtual int SimulateAttrGetinheritsched(void * ret_addr, const pthread_attr_t *, int *);
        virtual void AfterAttrGetinheritsched(void * ret_addr, int, const pthread_attr_t *, int *);

        virtual bool BeforeAttrSetstack(void * ret_addr, pthread_attr_t *, void *, size_t);
        virtual int SimulateAttrSetstack(void * ret_addr, pthread_attr_t *, void *, size_t);
        virtual void AfterAttrSetstack(void * ret_addr, int, pthread_attr_t *, void *, size_t);

        virtual bool BeforeCondSignal(void * ret_addr, pthread_cond_t *);
        virtual int SimulateCondSignal(void * ret_addr, pthread_cond_t *);
        virtual void AfterCondSignal(void * ret_addr, int, pthread_cond_t *);

        virtual bool BeforeSemGetvalue(void * ret_addr, sem_t *, int *);
        virtual int SimulateSemGetvalue(void * ret_addr, sem_t *, int *);
        virtual void AfterSemGetvalue(void * ret_addr, int, sem_t *, int *);

        virtual bool BeforeMutexattrInit(void * ret_addr, pthread_mutexattr_t *);
        virtual int SimulateMutexattrInit(void * ret_addr, pthread_mutexattr_t *);
        virtual void AfterMutexattrInit(void * ret_addr, int, pthread_mutexattr_t *);

        virtual bool BeforeBarrierWait(void * ret_addr, pthread_barrier_t *);
        virtual int SimulateBarrierWait(void * ret_addr, pthread_barrier_t *);
        virtual void AfterBarrierWait(void * ret_addr, int, pthread_barrier_t *);

        virtual bool BeforeSpinDestroy(void * ret_addr, pthread_spinlock_t *);
        virtual int SimulateSpinDestroy(void * ret_addr, pthread_spinlock_t *);
        virtual void AfterSpinDestroy(void * ret_addr, int, pthread_spinlock_t *);

        virtual bool BeforeKeyDelete(void * ret_addr, pthread_key_t);
        virtual int SimulateKeyDelete(void * ret_addr, pthread_key_t);
        virtual void AfterKeyDelete(void * ret_addr, int, pthread_key_t);

        virtual bool BeforeSetspecific(void * ret_addr, pthread_key_t, const void *);
        virtual int SimulateSetspecific(void * ret_addr, pthread_key_t, const void *);
        virtual void AfterSetspecific(void * ret_addr, int, pthread_key_t, const void *);

        virtual bool BeforeAttrSetstacksize(void * ret_addr, pthread_attr_t *, size_t);
        virtual int SimulateAttrSetstacksize(void * ret_addr, pthread_attr_t *, size_t);
        virtual void AfterAttrSetstacksize(void * ret_addr, int, pthread_attr_t *, size_t);

        virtual bool BeforeSetcancelstate(void * ret_addr, int, int *);
        virtual int SimulateSetcancelstate(void * ret_addr, int, int *);
        virtual void AfterSetcancelstate(void * ret_addr, int, int, int *);

        virtual bool BeforeBarrierattrSetpshared(void * ret_addr, pthread_barrierattr_t *, int);
        virtual int SimulateBarrierattrSetpshared(void * ret_addr, pthread_barrierattr_t *, int);
        virtual void AfterBarrierattrSetpshared(void * ret_addr, int, pthread_barrierattr_t *, int);

        virtual bool BeforeRwlockTimedrdlock(void * ret_addr, pthread_rwlock_t *, const struct timespec *);
        virtual int SimulateRwlockTimedrdlock(void * ret_addr, pthread_rwlock_t *, const struct timespec *);
        virtual void AfterRwlockTimedrdlock(void * ret_addr, int, pthread_rwlock_t *, const struct timespec *);

        virtual bool BeforeDetach(void * ret_addr, pthread_t);
        virtual int SimulateDetach(void * ret_addr, pthread_t);
        virtual void AfterDetach(void * ret_addr, int, pthread_t);

        virtual bool BeforeUsleep(void * ret_addr, useconds_t);
        virtual int SimulateUsleep(void * ret_addr, useconds_t);
        virtual void AfterUsleep(void * ret_addr, int, useconds_t);

        virtual bool BeforeRwlockDestroy(void * ret_addr, pthread_rwlock_t *);
        virtual int SimulateRwlockDestroy(void * ret_addr, pthread_rwlock_t *);
        virtual void AfterRwlockDestroy(void * ret_addr, int, pthread_rwlock_t *);

        virtual bool BeforeExit(void * ret_addr, void *);
        virtual void SimulateExit(void * ret_addr, void *);
        virtual void AfterExit(void * ret_addr, void *);

        virtual bool BeforeAttrSetdetachstate(void * ret_addr, pthread_attr_t *, int);
        virtual int SimulateAttrSetdetachstate(void * ret_addr, pthread_attr_t *, int);
        virtual void AfterAttrSetdetachstate(void * ret_addr, int, pthread_attr_t *, int);

        virtual bool BeforeRwlockTrywrlock(void * ret_addr, pthread_rwlock_t *);
        virtual int SimulateRwlockTrywrlock(void * ret_addr, pthread_rwlock_t *);
        virtual void AfterRwlockTrywrlock(void * ret_addr, int, pthread_rwlock_t *);

        virtual bool BeforeAttrInit(void * ret_addr, pthread_attr_t *);
        virtual int SimulateAttrInit(void * ret_addr, pthread_attr_t *);
        virtual void AfterAttrInit(void * ret_addr, int, pthread_attr_t *);

        virtual bool BeforeAttrSetguardsize(void * ret_addr, pthread_attr_t *, size_t);
        virtual int SimulateAttrSetguardsize(void * ret_addr, pthread_attr_t *, size_t);
        virtual void AfterAttrSetguardsize(void * ret_addr, int, pthread_attr_t *, size_t);

        virtual bool BeforeCondattrSetpshared(void * ret_addr, pthread_condattr_t *, int);
        virtual int SimulateCondattrSetpshared(void * ret_addr, pthread_condattr_t *, int);
        virtual void AfterCondattrSetpshared(void * ret_addr, int, pthread_condattr_t *, int);

        virtual bool BeforeMutexSetprioceiling(void * ret_addr, pthread_mutex_t *, int, int *);
        virtual int SimulateMutexSetprioceiling(void * ret_addr, pthread_mutex_t *, int, int *);
        virtual void AfterMutexSetprioceiling(void * ret_addr, int, pthread_mutex_t *, int, int *);

        virtual bool BeforeMutexattrGetprotocol(void * ret_addr, const pthread_mutexattr_t *, int *);
        virtual int SimulateMutexattrGetprotocol(void * ret_addr, const pthread_mutexattr_t *, int *);
        virtual void AfterMutexattrGetprotocol(void * ret_addr, int, const pthread_mutexattr_t *, int *);

        virtual bool BeforeAttrGetstack(void * ret_addr, const pthread_attr_t *, void **, size_t *);
        virtual int SimulateAttrGetstack(void * ret_addr, const pthread_attr_t *, void **, size_t *);
        virtual void AfterAttrGetstack(void * ret_addr, int, const pthread_attr_t *, void **, size_t *);

        virtual bool BeforeCondattrInit(void * ret_addr, pthread_condattr_t *);
        virtual int SimulateCondattrInit(void * ret_addr, pthread_condattr_t *);
        virtual void AfterCondattrInit(void * ret_addr, int, pthread_condattr_t *);

        virtual bool BeforeMutexattrGettype(void * ret_addr, const pthread_mutexattr_t *, int *);
        virtual int SimulateMutexattrGettype(void * ret_addr, const pthread_mutexattr_t *, int *);
        virtual void AfterMutexattrGettype(void * ret_addr, int, const pthread_mutexattr_t *, int *);

        virtual bool BeforeSemPost(void * ret_addr, sem_t *);
        virtual int SimulateSemPost(void * ret_addr, sem_t *);
        virtual void AfterSemPost(void * ret_addr, int, sem_t *);

        virtual bool BeforeAttrGetstacksize(void * ret_addr, const pthread_attr_t *, size_t *);
        virtual int SimulateAttrGetstacksize(void * ret_addr, const pthread_attr_t *, size_t *);
        virtual void AfterAttrGetstacksize(void * ret_addr, int, const pthread_attr_t *, size_t *);

        virtual bool BeforeCreate(void * ret_addr, pthread_t *, const pthread_attr_t *, void *(*)(void *), void *, thrID&);
        virtual int SimulateCreate(void * ret_addr, pthread_t *, const pthread_attr_t *, void *(*)(void *), void *, thrID&);
        virtual void AfterCreate(void * ret_addr, int, pthread_t *, const pthread_attr_t *, void *(*)(void *), void *, thrID&);

        virtual bool BeforeSemTimedwait(void * ret_addr, sem_t *, const struct timespec *);
        virtual int SimulateSemTimedwait(void * ret_addr, sem_t *, const struct timespec *);
        virtual void AfterSemTimedwait(void * ret_addr, int, sem_t *, const struct timespec *);

        virtual bool BeforeEqual(void * ret_addr, pthread_t, pthread_t);
        virtual int SimulateEqual(void * ret_addr, pthread_t, pthread_t);
        virtual void AfterEqual(void * ret_addr, int, pthread_t, pthread_t);

        virtual bool BeforeSpinInit(void * ret_addr, pthread_spinlock_t *, int);
        virtual int SimulateSpinInit(void * ret_addr, pthread_spinlock_t *, int);
        virtual void AfterSpinInit(void * ret_addr, int, pthread_spinlock_t *, int);

        virtual bool BeforeSchedYield(void * ret_addr);
        virtual int SimulateSchedYield(void * ret_addr);
        virtual void AfterSchedYield(void * ret_addr, int);

        virtual bool BeforeKeyCreate(void * ret_addr, pthread_key_t *, void (*)(void *));
        virtual int SimulateKeyCreate(void * ret_addr, pthread_key_t *, void (*)(void *));
        virtual void AfterKeyCreate(void * ret_addr, int, pthread_key_t *, void (*)(void *));

        virtual bool BeforeMutexattrDestroy(void * ret_addr, pthread_mutexattr_t *);
        virtual int SimulateMutexattrDestroy(void * ret_addr, pthread_mutexattr_t *);
        virtual void AfterMutexattrDestroy(void * ret_addr, int, pthread_mutexattr_t *);

        virtual bool BeforeCondTimedwait(void * ret_addr, pthread_cond_t *, pthread_mutex_t *, const struct timespec *);
        virtual int SimulateCondTimedwait(void * ret_addr, pthread_cond_t *, pthread_mutex_t *, const struct timespec *);
        virtual void AfterCondTimedwait(void * ret_addr, int, pthread_cond_t *, pthread_mutex_t *, const struct timespec *);

        virtual bool BeforeCondattrGetclock(void * ret_addr, const pthread_condattr_t *, clockid_t *);
        virtual int SimulateCondattrGetclock(void * ret_addr, const pthread_condattr_t *, clockid_t *);
        virtual void AfterCondattrGetclock(void * ret_addr, int, const pthread_condattr_t *, clockid_t *);

        virtual bool BeforeSetschedparam(void * ret_addr, pthread_t, int, const struct sched_param *);
        virtual int SimulateSetschedparam(void * ret_addr, pthread_t, int, const struct sched_param *);
        virtual void AfterSetschedparam(void * ret_addr, int, pthread_t, int, const struct sched_param *);

        virtual bool BeforeAttrSetinheritsched(void * ret_addr, pthread_attr_t *, int);
        virtual int SimulateAttrSetinheritsched(void * ret_addr, pthread_attr_t *, int);
        virtual void AfterAttrSetinheritsched(void * ret_addr, int, pthread_attr_t *, int);

        virtual bool BeforeMutexUnlock(void * ret_addr, pthread_mutex_t *);
        virtual int SimulateMutexUnlock(void * ret_addr, pthread_mutex_t *);
        virtual void AfterMutexUnlock(void * ret_addr, int, pthread_mutex_t *);

        virtual bool BeforeAttrSetschedparam(void * ret_addr, pthread_attr_t *, const struct sched_param *);
        virtual int SimulateAttrSetschedparam(void * ret_addr, pthread_attr_t *, const struct sched_param *);
        virtual void AfterAttrSetschedparam(void * ret_addr, int, pthread_attr_t *, const struct sched_param *);

        virtual bool BeforeRwlockattrSetpshared(void * ret_addr, pthread_rwlockattr_t *, int);
        virtual int SimulateRwlockattrSetpshared(void * ret_addr, pthread_rwlockattr_t *, int);
        virtual void AfterRwlockattrSetpshared(void * ret_addr, int, pthread_rwlockattr_t *, int);

        virtual bool BeforeAttrSetstackaddr(void * ret_addr, pthread_attr_t *, void *);
        virtual int SimulateAttrSetstackaddr(void * ret_addr, pthread_attr_t *, void *);
        virtual void AfterAttrSetstackaddr(void * ret_addr, int, pthread_attr_t *, void *);

        virtual bool BeforeMutexDestroy(void * ret_addr, pthread_mutex_t *);
        virtual int SimulateMutexDestroy(void * ret_addr, pthread_mutex_t *);
        virtual void AfterMutexDestroy(void * ret_addr, int, pthread_mutex_t *);

        virtual bool BeforeAttrGetschedpolicy(void * ret_addr, const pthread_attr_t *, int *);
        virtual int SimulateAttrGetschedpolicy(void * ret_addr, const pthread_attr_t *, int *);
        virtual void AfterAttrGetschedpolicy(void * ret_addr, int, const pthread_attr_t *, int *);

        virtual bool BeforeSetcanceltype(void * ret_addr, int, int *);
        virtual int SimulateSetcanceltype(void * ret_addr, int, int *);
        virtual void AfterSetcanceltype(void * ret_addr, int, int, int *);

        virtual bool BeforeSigwait(void * ret_addr, const sigset_t *, int *);
        virtual int SimulateSigwait(void * ret_addr, const sigset_t *, int *);
        virtual void AfterSigwait(void * ret_addr, int, const sigset_t *, int *);

        virtual bool BeforeRwlockInit(void * ret_addr, pthread_rwlock_t *, const pthread_rwlockattr_t *);
        virtual int SimulateRwlockInit(void * ret_addr, pthread_rwlock_t *, const pthread_rwlockattr_t *);
        virtual void AfterRwlockInit(void * ret_addr, int, pthread_rwlock_t *, const pthread_rwlockattr_t *);

        virtual bool BeforeRwlockUnlock(void * ret_addr, pthread_rwlock_t *);
        virtual int SimulateRwlockUnlock(void * ret_addr, pthread_rwlock_t *);
        virtual void AfterRwlockUnlock(void * ret_addr, int, pthread_rwlock_t *);

        virtual bool BeforeMutexattrSetprotocol(void * ret_addr, pthread_mutexattr_t *, int);
        virtual int SimulateMutexattrSetprotocol(void * ret_addr, pthread_mutexattr_t *, int);
        virtual void AfterMutexattrSetprotocol(void * ret_addr, int, pthread_mutexattr_t *, int);

        virtual bool BeforeMutexGetprioceiling(void * ret_addr, const pthread_mutex_t *, int *);
        virtual int SimulateMutexGetprioceiling(void * ret_addr, const pthread_mutex_t *, int *);
        virtual void AfterMutexGetprioceiling(void * ret_addr, int, const pthread_mutex_t *, int *);

        virtual bool BeforeSemTrywait(void * ret_addr, sem_t *);
        virtual int SimulateSemTrywait(void * ret_addr, sem_t *);
        virtual void AfterSemTrywait(void * ret_addr, int, sem_t *);

        virtual bool BeforeAttrGetstackaddr(void * ret_addr, const pthread_attr_t *, void **);
        virtual int SimulateAttrGetstackaddr(void * ret_addr, const pthread_attr_t *, void **);
        virtual void AfterAttrGetstackaddr(void * ret_addr, int, const pthread_attr_t *, void **);

        virtual bool BeforeMutexattrSettype(void * ret_addr, pthread_mutexattr_t *, int);
        virtual int SimulateMutexattrSettype(void * ret_addr, pthread_mutexattr_t *, int);
        virtual void AfterMutexattrSettype(void * ret_addr, int, pthread_mutexattr_t *, int);

        virtual bool BeforeGetcpuclockid(void * ret_addr, pthread_t, clockid_t *);
        virtual int SimulateGetcpuclockid(void * ret_addr, pthread_t, clockid_t *);
        virtual void AfterGetcpuclockid(void * ret_addr, int, pthread_t, clockid_t *);

        virtual bool BeforeCondDestroy(void * ret_addr, pthread_cond_t *);
        virtual int SimulateCondDestroy(void * ret_addr, pthread_cond_t *);
        virtual void AfterCondDestroy(void * ret_addr, int, pthread_cond_t *);

        virtual bool BeforeSpinLock(void * ret_addr, pthread_spinlock_t *);
        virtual int SimulateSpinLock(void * ret_addr, pthread_spinlock_t *);
        virtual void AfterSpinLock(void * ret_addr, int, pthread_spinlock_t *);

        virtual bool BeforeOnce(void * ret_addr, pthread_once_t *, void (*)(void));
        virtual int SimulateOnce(void * ret_addr, pthread_once_t *, void (*)(void));
        virtual void AfterOnce(void * ret_addr, int, pthread_once_t *, void (*)(void));

        virtual bool BeforeAtfork(void * ret_addr, void (*)(void), void (*)(void), void(*)(void));
        virtual int SimulateAtfork(void * ret_addr, void (*)(void), void (*)(void), void(*)(void));
        virtual void AfterAtfork(void * ret_addr, int, void (*)(void), void (*)(void), void(*)(void));

        virtual bool BeforeAttrSetscope(void * ret_addr, pthread_attr_t *, int);
        virtual int SimulateAttrSetscope(void * ret_addr, pthread_attr_t *, int);
        virtual void AfterAttrSetscope(void * ret_addr, int, pthread_attr_t *, int);

        virtual bool BeforeGetspecific(void * ret_addr, pthread_key_t);
        virtual void * SimulateGetspecific(void * ret_addr, pthread_key_t);
        virtual void AfterGetspecific(void * ret_addr, void *, pthread_key_t);

        virtual bool BeforeAttrDestroy(void * ret_addr, pthread_attr_t *);
        virtual int SimulateAttrDestroy(void * ret_addr, pthread_attr_t *);
        virtual void AfterAttrDestroy(void * ret_addr, int, pthread_attr_t *);

        virtual bool BeforeMutexTimedlock(void * ret_addr, pthread_mutex_t *, const struct timespec *);
        virtual int SimulateMutexTimedlock(void * ret_addr, pthread_mutex_t *, const struct timespec *);
        virtual void AfterMutexTimedlock(void * ret_addr, int, pthread_mutex_t *, const struct timespec *);

        virtual bool BeforeBarrierInit(void * ret_addr, pthread_barrier_t *, const pthread_barrierattr_t *, unsigned);
        virtual int SimulateBarrierInit(void * ret_addr, pthread_barrier_t *, const pthread_barrierattr_t *, unsigned);
        virtual void AfterBarrierInit(void * ret_addr, int, pthread_barrier_t *, const pthread_barrierattr_t *, unsigned);

        virtual bool BeforeSetschedprio(void * ret_addr, pthread_t, int);
        virtual int SimulateSetschedprio(void * ret_addr, pthread_t, int);
        virtual void AfterSetschedprio(void * ret_addr, int, pthread_t, int);

        virtual bool BeforeCondattrSetclock(void * ret_addr, pthread_condattr_t *, clockid_t);
        virtual int SimulateCondattrSetclock(void * ret_addr, pthread_condattr_t *, clockid_t);
        virtual void AfterCondattrSetclock(void * ret_addr, int, pthread_condattr_t *, clockid_t);

        virtual bool BeforeCondBroadcast(void * ret_addr, pthread_cond_t *);
        virtual int SimulateCondBroadcast(void * ret_addr, pthread_cond_t *);
        virtual void AfterCondBroadcast(void * ret_addr, int, pthread_cond_t *);

        virtual bool BeforeSpinUnlock(void * ret_addr, pthread_spinlock_t *);
        virtual int SimulateSpinUnlock(void * ret_addr, pthread_spinlock_t *);
        virtual void AfterSpinUnlock(void * ret_addr, int, pthread_spinlock_t *);

        virtual bool BeforeRwlockRdlock(void * ret_addr, pthread_rwlock_t *);
        virtual int SimulateRwlockRdlock(void * ret_addr, pthread_rwlock_t *);
        virtual void AfterRwlockRdlock(void * ret_addr, int, pthread_rwlock_t *);

        virtual bool BeforeRwlockWrlock(void * ret_addr, pthread_rwlock_t *);
        virtual int SimulateRwlockWrlock(void * ret_addr, pthread_rwlock_t *);
        virtual void AfterRwlockWrlock(void * ret_addr, int, pthread_rwlock_t *);

        virtual bool BeforeJoin(void * ret_addr, pthread_t, void **);
        virtual int SimulateJoin(void * ret_addr, pthread_t, void **);
        virtual void AfterJoin(void * ret_addr, int, pthread_t, void **);

        virtual bool BeforeCondInit(void * ret_addr, pthread_cond_t *, const pthread_condattr_t *);
        virtual int SimulateCondInit(void * ret_addr, pthread_cond_t *, const pthread_condattr_t *);
        virtual void AfterCondInit(void * ret_addr, int, pthread_cond_t *, const pthread_condattr_t *);

        virtual bool BeforeAttrGetguardsize(void * ret_addr, const pthread_attr_t *, size_t *);
        virtual int SimulateAttrGetguardsize(void * ret_addr, const pthread_attr_t *, size_t *);
        virtual void AfterAttrGetguardsize(void * ret_addr, int, const pthread_attr_t *, size_t *);

        virtual bool BeforeBarrierattrInit(void * ret_addr, pthread_barrierattr_t *);
        virtual int SimulateBarrierattrInit(void * ret_addr, pthread_barrierattr_t *);
        virtual void AfterBarrierattrInit(void * ret_addr, int, pthread_barrierattr_t *);

        virtual bool BeforeRwlockTryrdlock(void * ret_addr, pthread_rwlock_t *);
        virtual int SimulateRwlockTryrdlock(void * ret_addr, pthread_rwlock_t *);
        virtual void AfterRwlockTryrdlock(void * ret_addr, int, pthread_rwlock_t *);

        virtual bool BeforeAttrSetschedpolicy(void * ret_addr, pthread_attr_t *, int);
        virtual int SimulateAttrSetschedpolicy(void * ret_addr, pthread_attr_t *, int);
        virtual void AfterAttrSetschedpolicy(void * ret_addr, int, pthread_attr_t *, int);

        virtual bool BeforeBarrierattrDestroy(void * ret_addr, pthread_barrierattr_t *);
        virtual int SimulateBarrierattrDestroy(void * ret_addr, pthread_barrierattr_t *);
        virtual void AfterBarrierattrDestroy(void * ret_addr, int, pthread_barrierattr_t *);

        virtual bool BeforeSelf(void * ret_addr);
        virtual pthread_t SimulateSelf(void * ret_addr);
        virtual void AfterSelf(void * ret_addr, pthread_t);

        virtual bool BeforeAttrGetdetachstate(void * ret_addr, const pthread_attr_t *, int *);
        virtual int SimulateAttrGetdetachstate(void * ret_addr, const pthread_attr_t *, int *);
        virtual void AfterAttrGetdetachstate(void * ret_addr, int, const pthread_attr_t *, int *);

        virtual bool BeforeRwlockTimedwrlock(void * ret_addr, pthread_rwlock_t *, const struct timespec *);
        virtual int SimulateRwlockTimedwrlock(void * ret_addr, pthread_rwlock_t *, const struct timespec *);
        virtual void AfterRwlockTimedwrlock(void * ret_addr, int, pthread_rwlock_t *, const struct timespec *);

        virtual bool BeforeMutexattrSetprioceiling(void * ret_addr, pthread_mutexattr_t *, int);
        virtual int SimulateMutexattrSetprioceiling(void * ret_addr, pthread_mutexattr_t *, int);
        virtual void AfterMutexattrSetprioceiling(void * ret_addr, int, pthread_mutexattr_t *, int);


    private:

        int sem_wait (void * ret_addr, sem_t *);
        int thrille_mutex_lock (void * ret_addr, pthread_mutex_t *);
        unsigned int sleep (void * ret_addr, unsigned int);
        int thrille_condattr_destroy (void * ret_addr, pthread_condattr_t *);
        int thrille_mutexattr_getpshared (void * ret_addr, const pthread_mutexattr_t *, int *);
        int thrille_setconcurrency (void * ret_addr, int);
        int thrille_getconcurrency (void * ret_addr);
        int sem_destroy (void * ret_addr, sem_t *);
        void thrille_testcancel (void * ret_addr);
        int thrille_attr_getscope (void * ret_addr, const pthread_attr_t *, int *);
        int thrille_mutex_init (void * ret_addr, pthread_mutex_t *, const pthread_mutexattr_t *);
        int sem_init (void * ret_addr, sem_t *, int, unsigned int);
        int thrille_mutex_trylock (void * ret_addr, pthread_mutex_t *);
        int thrille_rwlockattr_init (void * ret_addr, pthread_rwlockattr_t *);
        int thrille_mutexattr_setpshared (void * ret_addr, pthread_mutexattr_t *, int);
        int thrille_rwlockattr_destroy (void * ret_addr, pthread_rwlockattr_t *);
        int thrille_barrier_destroy (void * ret_addr, pthread_barrier_t *);
        int thrille_getschedparam (void * ret_addr, pthread_t, int *, struct sched_param *);
        int thrille_attr_getschedparam (void * ret_addr, const pthread_attr_t *, struct sched_param *);
        int thrille_cancel (void * ret_addr, pthread_t);
        int thrille_cond_wait (void * ret_addr, pthread_cond_t *, pthread_mutex_t *);
        int thrille_condattr_getpshared (void * ret_addr, const pthread_condattr_t *, int *);
        int thrille_spin_trylock (void * ret_addr, pthread_spinlock_t *);
        int thrille_attr_getinheritsched (void * ret_addr, const pthread_attr_t *, int *);
        int thrille_attr_setstack (void * ret_addr, pthread_attr_t *, void *, size_t);
        int thrille_cond_signal (void * ret_addr, pthread_cond_t *);
        int sem_getvalue (void * ret_addr, sem_t *, int *);
        int thrille_mutexattr_init (void * ret_addr, pthread_mutexattr_t *);
        int thrille_barrier_wait (void * ret_addr, pthread_barrier_t *);
        int thrille_spin_destroy (void * ret_addr, pthread_spinlock_t *);
        int thrille_key_delete (void * ret_addr, pthread_key_t);
        int thrille_setspecific (void * ret_addr, pthread_key_t, const void *);
        int thrille_attr_setstacksize (void * ret_addr, pthread_attr_t *, size_t);
        int thrille_setcancelstate (void * ret_addr, int, int *);
        int thrille_barrierattr_setpshared (void * ret_addr, pthread_barrierattr_t *, int);
        int thrille_rwlock_timedrdlock (void * ret_addr, pthread_rwlock_t *, const struct timespec *);
        int thrille_detach (void * ret_addr, pthread_t);
        int usleep (void * ret_addr, useconds_t);
        int thrille_rwlock_destroy (void * ret_addr, pthread_rwlock_t *);
        void thrille_exit (void * ret_addr, void *);
        int thrille_attr_setdetachstate (void * ret_addr, pthread_attr_t *, int);
        int thrille_rwlock_trywrlock (void * ret_addr, pthread_rwlock_t *);
        int thrille_attr_init (void * ret_addr, pthread_attr_t *);
        int thrille_attr_setguardsize (void * ret_addr, pthread_attr_t *, size_t);
        int thrille_condattr_setpshared (void * ret_addr, pthread_condattr_t *, int);
        int thrille_mutex_setprioceiling (void * ret_addr, pthread_mutex_t *, int, int *);
        int thrille_mutexattr_getprotocol (void * ret_addr, const pthread_mutexattr_t *, int *);
        int thrille_attr_getstack (void * ret_addr, const pthread_attr_t *, void **, size_t *);
        int thrille_condattr_init (void * ret_addr, pthread_condattr_t *);
        int thrille_mutexattr_gettype (void * ret_addr, const pthread_mutexattr_t *, int *);
        int sem_post (void * ret_addr, sem_t *);
        int thrille_attr_getstacksize (void * ret_addr, const pthread_attr_t *, size_t *);
        int thrille_create (void * ret_addr, pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
        int sem_timedwait (void * ret_addr, sem_t *, const struct timespec *);
        int thrille_equal (void * ret_addr, pthread_t, pthread_t);
        int thrille_spin_init (void * ret_addr, pthread_spinlock_t *, int);
        int sched_yield (void * ret_addr);
        int thrille_key_create (void * ret_addr, pthread_key_t *, void (*)(void *));
        int thrille_mutexattr_destroy (void * ret_addr, pthread_mutexattr_t *);
        int thrille_cond_timedwait (void * ret_addr, pthread_cond_t *, pthread_mutex_t *, const struct timespec *);
        int thrille_condattr_getclock (void * ret_addr, const pthread_condattr_t *, clockid_t *);
        int thrille_setschedparam (void * ret_addr, pthread_t, int, const struct sched_param *);
        int thrille_attr_setinheritsched (void * ret_addr, pthread_attr_t *, int);
        int thrille_mutex_unlock (void * ret_addr, pthread_mutex_t *);
        int thrille_attr_setschedparam (void * ret_addr, pthread_attr_t *, const struct sched_param *);
        int thrille_rwlockattr_setpshared (void * ret_addr, pthread_rwlockattr_t *, int);
        int thrille_attr_setstackaddr (void * ret_addr, pthread_attr_t *, void *);
        int thrille_mutex_destroy (void * ret_addr, pthread_mutex_t *);
        int thrille_attr_getschedpolicy (void * ret_addr, const pthread_attr_t *, int *);
        int thrille_setcanceltype (void * ret_addr, int, int *);
        int sigwait (void * ret_addr, const sigset_t *, int *);
        int thrille_rwlock_init (void * ret_addr, pthread_rwlock_t *, const pthread_rwlockattr_t *);
        int thrille_rwlock_unlock (void * ret_addr, pthread_rwlock_t *);
        int thrille_mutexattr_setprotocol (void * ret_addr, pthread_mutexattr_t *, int);
        int thrille_mutex_getprioceiling (void * ret_addr, const pthread_mutex_t *, int *);
        int sem_trywait (void * ret_addr, sem_t *);
        int thrille_attr_getstackaddr (void * ret_addr, const pthread_attr_t *, void **);
        int thrille_mutexattr_settype (void * ret_addr, pthread_mutexattr_t *, int);
        int thrille_getcpuclockid (void * ret_addr, pthread_t, clockid_t *);
        int thrille_cond_destroy (void * ret_addr, pthread_cond_t *);
        int thrille_spin_lock (void * ret_addr, pthread_spinlock_t *);
        int thrille_once (void * ret_addr, pthread_once_t *, void (*)(void));
        int thrille_atfork (void * ret_addr, void (*)(void), void (*)(void), void(*)(void));
        int thrille_attr_setscope (void * ret_addr, pthread_attr_t *, int);
        void * thrille_getspecific (void * ret_addr, pthread_key_t);
        int thrille_attr_destroy (void * ret_addr, pthread_attr_t *);
        int thrille_mutex_timedlock (void * ret_addr, pthread_mutex_t *, const struct timespec *);
        int thrille_barrier_init (void * ret_addr, pthread_barrier_t *, const pthread_barrierattr_t *, unsigned);
        int thrille_setschedprio (void * ret_addr, pthread_t, int);
        int thrille_condattr_setclock (void * ret_addr, pthread_condattr_t *, clockid_t);
        int thrille_cond_broadcast (void * ret_addr, pthread_cond_t *);
        int thrille_spin_unlock (void * ret_addr, pthread_spinlock_t *);
        int thrille_rwlock_rdlock (void * ret_addr, pthread_rwlock_t *);
        int thrille_rwlock_wrlock (void * ret_addr, pthread_rwlock_t *);
        int thrille_join (void * ret_addr, pthread_t, void **);
        int thrille_cond_init (void * ret_addr, pthread_cond_t *, const pthread_condattr_t *);
        int thrille_attr_getguardsize (void * ret_addr, const pthread_attr_t *, size_t *);
        int thrille_barrierattr_init (void * ret_addr, pthread_barrierattr_t *);
        int thrille_rwlock_tryrdlock (void * ret_addr, pthread_rwlock_t *);
        int thrille_attr_setschedpolicy (void * ret_addr, pthread_attr_t *, int);
        int thrille_barrierattr_destroy (void * ret_addr, pthread_barrierattr_t *);
        pthread_t thrille_self (void * ret_addr);
        int thrille_attr_getdetachstate (void * ret_addr, const pthread_attr_t *, int *);
        int thrille_rwlock_timedwrlock (void * ret_addr, pthread_rwlock_t *, const struct timespec *);
        int thrille_mutexattr_setprioceiling (void * ret_addr, pthread_mutexattr_t *, int);

        friend void pth_start(void);
        friend void * my_start_routine(void * my_arg);

        friend int sem_wait(sem_t *);
        friend int pthread_mutex_lock(pthread_mutex_t *);
        friend unsigned int sleep(unsigned int);
        friend int pthread_condattr_destroy(pthread_condattr_t *);
        friend int pthread_mutexattr_getpshared(const pthread_mutexattr_t *, int *);
        friend int pthread_setconcurrency(int);
        friend int pthread_getconcurrency();
        friend int sem_destroy(sem_t *);
        friend void pthread_testcancel();
        friend int pthread_attr_getscope(const pthread_attr_t *, int *);
        friend int pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *);
        friend int sem_init(sem_t *, int, unsigned int);
        friend int pthread_mutex_trylock(pthread_mutex_t *);
        friend int pthread_rwlockattr_init(pthread_rwlockattr_t *);
        friend int pthread_mutexattr_setpshared(pthread_mutexattr_t *, int);
        friend int pthread_rwlockattr_destroy(pthread_rwlockattr_t *);
        friend int pthread_barrier_destroy(pthread_barrier_t *);
        friend int pthread_getschedparam(pthread_t, int *, struct sched_param *);
        friend int pthread_attr_getschedparam(const pthread_attr_t *, struct sched_param *);
        friend int pthread_cancel(pthread_t);
        friend int pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);
        friend int pthread_condattr_getpshared(const pthread_condattr_t *, int *);
        friend int pthread_spin_trylock(pthread_spinlock_t *);
        friend int pthread_attr_getinheritsched(const pthread_attr_t *, int *);
        friend int pthread_attr_setstack(pthread_attr_t *, void *, size_t);
        friend int pthread_cond_signal(pthread_cond_t *);
        friend int sem_getvalue(sem_t *, int *);
        friend int pthread_mutexattr_init(pthread_mutexattr_t *);
        friend int pthread_barrier_wait(pthread_barrier_t *);
        friend int pthread_spin_destroy(pthread_spinlock_t *);
        friend int pthread_key_delete(pthread_key_t);
        friend int pthread_setspecific(pthread_key_t, const void *);
        friend int pthread_attr_setstacksize(pthread_attr_t *, size_t);
        friend int pthread_setcancelstate(int, int *);
        friend int pthread_barrierattr_setpshared(pthread_barrierattr_t *, int);
        friend int pthread_rwlock_timedrdlock(pthread_rwlock_t *, const struct timespec *);
        friend int pthread_detach(pthread_t);
        friend int usleep(useconds_t);
        friend int pthread_rwlock_destroy(pthread_rwlock_t *);
        friend void pthread_exit(void *);
        friend int pthread_attr_setdetachstate(pthread_attr_t *, int);
        friend int pthread_rwlock_trywrlock(pthread_rwlock_t *);
        friend int pthread_attr_init(pthread_attr_t *);
        friend int pthread_attr_setguardsize(pthread_attr_t *, size_t);
        friend int pthread_condattr_setpshared(pthread_condattr_t *, int);
        friend int pthread_mutex_setprioceiling(pthread_mutex_t *, int, int *);
        friend int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *, int *);
        friend int pthread_attr_getstack(const pthread_attr_t *, void **, size_t *);
        friend int pthread_condattr_init(pthread_condattr_t *);
        friend int pthread_mutexattr_gettype(const pthread_mutexattr_t *, int *);
        friend int sem_post(sem_t *);
        friend int pthread_attr_getstacksize(const pthread_attr_t *, size_t *);
        friend int pthread_create(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
        friend int sem_timedwait(sem_t *, const struct timespec *);
        friend int pthread_equal(pthread_t, pthread_t);
        friend int pthread_spin_init(pthread_spinlock_t *, int);
        friend int sched_yield();
        friend int pthread_key_create(pthread_key_t *, void (*)(void *));
        friend int pthread_mutexattr_destroy(pthread_mutexattr_t *);
        friend int pthread_cond_timedwait(pthread_cond_t *, pthread_mutex_t *, const struct timespec *);
        friend int pthread_condattr_getclock(const pthread_condattr_t *, clockid_t *);
        friend int pthread_setschedparam(pthread_t, int, const struct sched_param *);
        friend int pthread_attr_setinheritsched(pthread_attr_t *, int);
        friend int pthread_mutex_unlock(pthread_mutex_t *);
        friend int pthread_attr_setschedparam(pthread_attr_t *, const struct sched_param *);
        friend int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *, int);
        friend int pthread_attr_setstackaddr(pthread_attr_t *, void *);
        friend int pthread_mutex_destroy(pthread_mutex_t *);
        friend int pthread_attr_getschedpolicy(const pthread_attr_t *, int *);
        friend int pthread_setcanceltype(int, int *);
        friend int sigwait(const sigset_t *, int *);
        friend int pthread_rwlock_init(pthread_rwlock_t *, const pthread_rwlockattr_t *);
        friend int pthread_rwlock_unlock(pthread_rwlock_t *);
        friend int pthread_mutexattr_setprotocol(pthread_mutexattr_t *, int);
        friend int pthread_mutex_getprioceiling(const pthread_mutex_t *, int *);
        friend int sem_trywait(sem_t *);
        friend int pthread_attr_getstackaddr(const pthread_attr_t *, void **);
        friend int pthread_mutexattr_settype(pthread_mutexattr_t *, int);
        friend int pthread_getcpuclockid(pthread_t, clockid_t *);
        friend int pthread_cond_destroy(pthread_cond_t *);
        friend int pthread_spin_lock(pthread_spinlock_t *);
        friend int pthread_once(pthread_once_t *, void (*)(void));
        friend int pthread_atfork(void (*)(void), void (*)(void), void(*)(void));
        friend int pthread_attr_setscope(pthread_attr_t *, int);
        friend void * pthread_getspecific(pthread_key_t);
        friend int pthread_attr_destroy(pthread_attr_t *);
        friend int pthread_mutex_timedlock(pthread_mutex_t *, const struct timespec *);
        friend int pthread_barrier_init(pthread_barrier_t *, const pthread_barrierattr_t *, unsigned);
        friend int pthread_setschedprio(pthread_t, int);
        friend int pthread_condattr_setclock(pthread_condattr_t *, clockid_t);
        friend int pthread_cond_broadcast(pthread_cond_t *);
        friend int pthread_spin_unlock(pthread_spinlock_t *);
        friend int pthread_rwlock_rdlock(pthread_rwlock_t *);
        friend int pthread_rwlock_wrlock(pthread_rwlock_t *);
        friend int pthread_join(pthread_t, void **);
        friend int pthread_cond_init(pthread_cond_t *, const pthread_condattr_t *);
        friend int pthread_attr_getguardsize(const pthread_attr_t *, size_t *);
        friend int pthread_barrierattr_init(pthread_barrierattr_t *);
        friend int pthread_rwlock_tryrdlock(pthread_rwlock_t *);
        friend int pthread_attr_setschedpolicy(pthread_attr_t *, int);
        friend int pthread_barrierattr_destroy(pthread_barrierattr_t *);
        friend pthread_t pthread_self();
        friend int pthread_attr_getdetachstate(const pthread_attr_t *, int *);
        friend int pthread_rwlock_timedwrlock(pthread_rwlock_t *, const struct timespec *);
        friend int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *, int);

};

extern Handler* create_handler();

extern Handler * volatile pHandler;


#endif
