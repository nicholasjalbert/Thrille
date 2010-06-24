#include "libpth.h"

Handler * volatile pHandler = NULL;
volatile bool Handler::pth_is_initialized = false;

void pth_start(void)
{
    if (! Handler::pth_is_initialized) {
        Originals::initialize();
        pHandler = create_handler();
        safe_assert(pHandler != NULL);
        pHandler->AfterInitialize();
        pHandler->initializationDone();
    }
}

void pth_end(void)
{
    delete pHandler;
}


struct args_t
{
    void *(*start_routine)(void*);
    void * arg;
    thrID tid;

    args_t(void *(*start_routine_)(void*),
            void * arg_,
            thrID tid_)
        : start_routine(start_routine_),
        arg(arg_),
        tid(tid_)
    {}

};

void * my_start_routine(void * my_arg)
{
    args_t * const arg = static_cast<args_t *>(my_arg);

    Originals::pthread_mutex_lock(&(pHandler->newthread_lock));
    
    pHandler->setTID(arg->tid, Originals::pthread_self());
    
    pHandler->thread_start_map[arg->tid] = true;
    Originals::pthread_cond_signal(&(pHandler->newthread_cond));
    Originals::pthread_mutex_unlock(&(pHandler->newthread_lock));

    pHandler->ThreadStart(arg->start_routine, arg->arg);

    void * status = arg->start_routine(arg->arg);

    pHandler->ThreadFinish(arg->start_routine, status);

    delete arg;

    return status;
}

//implementation of Thrille private methods

void Handler::initializationDone() {
    safe_assert(! pth_is_initialized);
    pth_is_initialized = true;
}

thrID Handler::getThrID() {
    return translateHandleToTID(Originals::pthread_self());
}

thrID Handler::translateHandleToTID(pthread_t handle) {
    return threadtracker->translateHandleToTID(handle);
}

void Handler::setTID(thrID tid, pthread_t handle) {
    threadtracker->setTID(tid, handle);
}

int Handler::thrille_create(void * ret_addr,
        pthread_t* newthread,
        const pthread_attr_t *attr, 
        void *(*start_routine) (void *),
        void *arg) {

    single_threaded = false;
    
    int ret_val;

    thrID child_id = threadtracker->getNextTID();

    bool call_original = BeforeCreate(ret_addr,
            newthread,
            attr,
            start_routine,
            arg,
            child_id);

    args_t * const my_arg = new args_t(start_routine, arg, child_id);
    
    Originals::pthread_mutex_lock(&newthread_lock);
    thread_start_map[child_id] = false;
    Originals::pthread_mutex_unlock(&newthread_lock);

    if (call_original) {
        Originals::pthread_mutex_lock(&newthread_lock);
        
        ret_val = Originals::pthread_create(newthread, 
                attr, 
                my_start_routine, 
                (void *)my_arg);

        while (!thread_start_map[child_id]) {
            Originals::pthread_cond_wait(&newthread_cond, &newthread_lock);
        }

        Originals::pthread_mutex_unlock(&newthread_lock);
    } else {
        ret_val = SimulateCreate(ret_addr,
                newthread, 
                attr, 
                start_routine, 
                arg,
                child_id); 
    }
    AfterCreate(ret_addr, ret_val, newthread, attr, start_routine, arg, child_id);
    return ret_val;
}

int Handler::sem_wait(void * ret_addr, sem_t * param0) {
    int ret_val;
    bool call_original = BeforeSemWait(ret_addr, param0);

    if (call_original)
        ret_val = Originals::sem_wait(param0);
    else
        ret_val = SimulateSemWait(ret_addr, param0);

    AfterSemWait(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_mutex_lock(void * ret_addr, pthread_mutex_t * param0) {
    int ret_val;
    bool call_original = BeforeMutexLock(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_mutex_lock(param0);
    else
        ret_val = SimulateMutexLock(ret_addr, param0);

    AfterMutexLock(ret_addr, ret_val, param0);
    return ret_val; 
}

unsigned int Handler::sleep(void * ret_addr, unsigned int param0) {
    unsigned int ret_val;
    bool call_original = BeforeSleep(ret_addr, param0);

    if (call_original)
        ret_val = Originals::sleep(param0);
    else
        ret_val = SimulateSleep(ret_addr, param0);

    AfterSleep(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_condattr_destroy(void * ret_addr, pthread_condattr_t * param0) {
    int ret_val;
    bool call_original = BeforeCondattrDestroy(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_condattr_destroy(param0);
    else
        ret_val = SimulateCondattrDestroy(ret_addr, param0);

    AfterCondattrDestroy(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_mutexattr_getpshared(void * ret_addr, const pthread_mutexattr_t * param0, int * param1) {
    int ret_val;
    bool call_original = BeforeMutexattrGetpshared(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_mutexattr_getpshared(param0, param1);
    else
        ret_val = SimulateMutexattrGetpshared(ret_addr, param0, param1);

    AfterMutexattrGetpshared(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_setconcurrency(void * ret_addr, int param0) {
    int ret_val;
    bool call_original = BeforeSetconcurrency(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_setconcurrency(param0);
    else
        ret_val = SimulateSetconcurrency(ret_addr, param0);

    AfterSetconcurrency(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_getconcurrency(void * ret_addr) {
    int ret_val;
    bool call_original = BeforeGetconcurrency(ret_addr);

    if (call_original)
        ret_val = Originals::pthread_getconcurrency();
    else
        ret_val = SimulateGetconcurrency(ret_addr);

    AfterGetconcurrency(ret_addr, ret_val);
    return ret_val; 
}

int Handler::sem_destroy(void * ret_addr, sem_t * param0) {
    int ret_val;
    bool call_original = BeforeSemDestroy(ret_addr, param0);

    if (call_original)
        ret_val = Originals::sem_destroy(param0);
    else
        ret_val = SimulateSemDestroy(ret_addr, param0);

    AfterSemDestroy(ret_addr, ret_val, param0);
    return ret_val; 
}

void Handler::thrille_testcancel(void * ret_addr) {
    bool call_original = BeforeTestcancel(ret_addr);

    if (call_original)
        Originals::pthread_testcancel();
    else
        SimulateTestcancel(ret_addr);

    AfterTestcancel(ret_addr);
}

int Handler::thrille_attr_getscope(void * ret_addr, const pthread_attr_t * param0, int * param1) {
    int ret_val;
    bool call_original = BeforeAttrGetscope(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_getscope(param0, param1);
    else
        ret_val = SimulateAttrGetscope(ret_addr, param0, param1);

    AfterAttrGetscope(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_mutex_init(void * ret_addr, pthread_mutex_t * param0, const pthread_mutexattr_t * param1) {
    int ret_val;
    bool call_original = BeforeMutexInit(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_mutex_init(param0, param1);
    else
        ret_val = SimulateMutexInit(ret_addr, param0, param1);

    AfterMutexInit(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::sem_init(void * ret_addr, sem_t * param0, int param1, unsigned int param2) {
    int ret_val;
    bool call_original = BeforeSemInit(ret_addr, param0, param1, param2);

    if (call_original)
        ret_val = Originals::sem_init(param0, param1, param2);
    else
        ret_val = SimulateSemInit(ret_addr, param0, param1, param2);

    AfterSemInit(ret_addr, ret_val, param0, param1, param2);
    return ret_val; 
}

int Handler::thrille_mutex_trylock(void * ret_addr, pthread_mutex_t * param0) {
    int ret_val;
    bool call_original = BeforeMutexTrylock(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_mutex_trylock(param0);
    else
        ret_val = SimulateMutexTrylock(ret_addr, param0);

    AfterMutexTrylock(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_rwlockattr_init(void * ret_addr, pthread_rwlockattr_t * param0) {
    int ret_val;
    bool call_original = BeforeRwlockattrInit(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_rwlockattr_init(param0);
    else
        ret_val = SimulateRwlockattrInit(ret_addr, param0);

    AfterRwlockattrInit(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_mutexattr_setpshared(void * ret_addr, pthread_mutexattr_t * param0, int param1) {
    int ret_val;
    bool call_original = BeforeMutexattrSetpshared(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_mutexattr_setpshared(param0, param1);
    else
        ret_val = SimulateMutexattrSetpshared(ret_addr, param0, param1);

    AfterMutexattrSetpshared(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_rwlockattr_destroy(void * ret_addr, pthread_rwlockattr_t * param0) {
    int ret_val;
    bool call_original = BeforeRwlockattrDestroy(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_rwlockattr_destroy(param0);
    else
        ret_val = SimulateRwlockattrDestroy(ret_addr, param0);

    AfterRwlockattrDestroy(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_barrier_destroy(void * ret_addr, pthread_barrier_t * param0) {
    int ret_val;
    bool call_original = BeforeBarrierDestroy(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_barrier_destroy(param0);
    else
        ret_val = SimulateBarrierDestroy(ret_addr, param0);

    AfterBarrierDestroy(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_getschedparam(void * ret_addr, pthread_t param0, int * param1, struct sched_param * param2) {
    int ret_val;
    bool call_original = BeforeGetschedparam(ret_addr, param0, param1, param2);

    if (call_original)
        ret_val = Originals::pthread_getschedparam(param0, param1, param2);
    else
        ret_val = SimulateGetschedparam(ret_addr, param0, param1, param2);

    AfterGetschedparam(ret_addr, ret_val, param0, param1, param2);
    return ret_val; 
}

int Handler::thrille_attr_getschedparam(void * ret_addr, const pthread_attr_t * param0, struct sched_param * param1) {
    int ret_val;
    bool call_original = BeforeAttrGetschedparam(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_getschedparam(param0, param1);
    else
        ret_val = SimulateAttrGetschedparam(ret_addr, param0, param1);

    AfterAttrGetschedparam(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_cancel(void * ret_addr, pthread_t param0) {
    int ret_val;
    bool call_original = BeforeCancel(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_cancel(param0);
    else
        ret_val = SimulateCancel(ret_addr, param0);

    AfterCancel(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_cond_wait(void * ret_addr, pthread_cond_t * param0, pthread_mutex_t * param1) {
    int ret_val;
    bool call_original = BeforeCondWait(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_cond_wait(param0, param1);
    else
        ret_val = SimulateCondWait(ret_addr, param0, param1);

    AfterCondWait(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_condattr_getpshared(void * ret_addr, const pthread_condattr_t * param0, int * param1) {
    int ret_val;
    bool call_original = BeforeCondattrGetpshared(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_condattr_getpshared(param0, param1);
    else
        ret_val = SimulateCondattrGetpshared(ret_addr, param0, param1);

    AfterCondattrGetpshared(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_spin_trylock(void * ret_addr, pthread_spinlock_t * param0) {
    int ret_val;
    bool call_original = BeforeSpinTrylock(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_spin_trylock(param0);
    else
        ret_val = SimulateSpinTrylock(ret_addr, param0);

    AfterSpinTrylock(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_attr_getinheritsched(void * ret_addr, const pthread_attr_t * param0, int * param1) {
    int ret_val;
    bool call_original = BeforeAttrGetinheritsched(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_getinheritsched(param0, param1);
    else
        ret_val = SimulateAttrGetinheritsched(ret_addr, param0, param1);

    AfterAttrGetinheritsched(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_attr_setstack(void * ret_addr, pthread_attr_t * param0, void * param1, size_t param2) {
    int ret_val;
    bool call_original = BeforeAttrSetstack(ret_addr, param0, param1, param2);

    if (call_original)
        ret_val = Originals::pthread_attr_setstack(param0, param1, param2);
    else
        ret_val = SimulateAttrSetstack(ret_addr, param0, param1, param2);

    AfterAttrSetstack(ret_addr, ret_val, param0, param1, param2);
    return ret_val; 
}

int Handler::thrille_cond_signal(void * ret_addr, pthread_cond_t * param0) {
    int ret_val;
    bool call_original = BeforeCondSignal(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_cond_signal(param0);
    else
        ret_val = SimulateCondSignal(ret_addr, param0);

    AfterCondSignal(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::sem_getvalue(void * ret_addr, sem_t * param0, int * param1) {
    int ret_val;
    bool call_original = BeforeSemGetvalue(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::sem_getvalue(param0, param1);
    else
        ret_val = SimulateSemGetvalue(ret_addr, param0, param1);

    AfterSemGetvalue(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_mutexattr_init(void * ret_addr, pthread_mutexattr_t * param0) {
    int ret_val;
    bool call_original = BeforeMutexattrInit(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_mutexattr_init(param0);
    else
        ret_val = SimulateMutexattrInit(ret_addr, param0);

    AfterMutexattrInit(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_barrier_wait(void * ret_addr, pthread_barrier_t * param0) {
    int ret_val;
    bool call_original = BeforeBarrierWait(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_barrier_wait(param0);
    else
        ret_val = SimulateBarrierWait(ret_addr, param0);

    AfterBarrierWait(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_spin_destroy(void * ret_addr, pthread_spinlock_t * param0) {
    int ret_val;
    bool call_original = BeforeSpinDestroy(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_spin_destroy(param0);
    else
        ret_val = SimulateSpinDestroy(ret_addr, param0);

    AfterSpinDestroy(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_key_delete(void * ret_addr, pthread_key_t param0) {
    int ret_val;
    bool call_original = BeforeKeyDelete(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_key_delete(param0);
    else
        ret_val = SimulateKeyDelete(ret_addr, param0);

    AfterKeyDelete(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_setspecific(void * ret_addr, pthread_key_t param0, const void * param1) {
    int ret_val;
    bool call_original = BeforeSetspecific(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_setspecific(param0, param1);
    else
        ret_val = SimulateSetspecific(ret_addr, param0, param1);

    AfterSetspecific(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_attr_setstacksize(void * ret_addr, pthread_attr_t * param0, size_t param1) {
    int ret_val;
    bool call_original = BeforeAttrSetstacksize(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_setstacksize(param0, param1);
    else
        ret_val = SimulateAttrSetstacksize(ret_addr, param0, param1);

    AfterAttrSetstacksize(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_setcancelstate(void * ret_addr, int param0, int * param1) {
    int ret_val;
    bool call_original = BeforeSetcancelstate(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_setcancelstate(param0, param1);
    else
        ret_val = SimulateSetcancelstate(ret_addr, param0, param1);

    AfterSetcancelstate(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_barrierattr_setpshared(void * ret_addr, pthread_barrierattr_t * param0, int param1) {
    int ret_val;
    bool call_original = BeforeBarrierattrSetpshared(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_barrierattr_setpshared(param0, param1);
    else
        ret_val = SimulateBarrierattrSetpshared(ret_addr, param0, param1);

    AfterBarrierattrSetpshared(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_rwlock_timedrdlock(void * ret_addr, pthread_rwlock_t * param0, const struct timespec * param1) {
    int ret_val;
    bool call_original = BeforeRwlockTimedrdlock(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_rwlock_timedrdlock(param0, param1);
    else
        ret_val = SimulateRwlockTimedrdlock(ret_addr, param0, param1);

    AfterRwlockTimedrdlock(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_detach(void * ret_addr, pthread_t param0) {
    int ret_val;
    bool call_original = BeforeDetach(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_detach(param0);
    else
        ret_val = SimulateDetach(ret_addr, param0);

    AfterDetach(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::usleep(void * ret_addr, useconds_t param0) {
    int ret_val;
    bool call_original = BeforeUsleep(ret_addr, param0);

    if (call_original)
        ret_val = Originals::usleep(param0);
    else
        ret_val = SimulateUsleep(ret_addr, param0);

    AfterUsleep(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_rwlock_destroy(void * ret_addr, pthread_rwlock_t * param0) {
    int ret_val;
    bool call_original = BeforeRwlockDestroy(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_rwlock_destroy(param0);
    else
        ret_val = SimulateRwlockDestroy(ret_addr, param0);

    AfterRwlockDestroy(ret_addr, ret_val, param0);
    return ret_val; 
}

void Handler::thrille_exit(void * ret_addr, void * param0) {
    bool call_original = BeforeExit(ret_addr, param0);

    if (call_original)
        Originals::pthread_exit(param0);
    else
        SimulateExit(ret_addr, param0);

    AfterExit(ret_addr, param0);
}

int Handler::thrille_attr_setdetachstate(void * ret_addr, pthread_attr_t * param0, int param1) {
    int ret_val;
    bool call_original = BeforeAttrSetdetachstate(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_setdetachstate(param0, param1);
    else
        ret_val = SimulateAttrSetdetachstate(ret_addr, param0, param1);

    AfterAttrSetdetachstate(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_rwlock_trywrlock(void * ret_addr, pthread_rwlock_t * param0) {
    int ret_val;
    bool call_original = BeforeRwlockTrywrlock(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_rwlock_trywrlock(param0);
    else
        ret_val = SimulateRwlockTrywrlock(ret_addr, param0);

    AfterRwlockTrywrlock(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_attr_init(void * ret_addr, pthread_attr_t * param0) {
    int ret_val;
    bool call_original = BeforeAttrInit(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_attr_init(param0);
    else
        ret_val = SimulateAttrInit(ret_addr, param0);

    AfterAttrInit(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_attr_setguardsize(void * ret_addr, pthread_attr_t * param0, size_t param1) {
    int ret_val;
    bool call_original = BeforeAttrSetguardsize(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_setguardsize(param0, param1);
    else
        ret_val = SimulateAttrSetguardsize(ret_addr, param0, param1);

    AfterAttrSetguardsize(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_condattr_setpshared(void * ret_addr, pthread_condattr_t * param0, int param1) {
    int ret_val;
    bool call_original = BeforeCondattrSetpshared(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_condattr_setpshared(param0, param1);
    else
        ret_val = SimulateCondattrSetpshared(ret_addr, param0, param1);

    AfterCondattrSetpshared(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_mutex_setprioceiling(void * ret_addr, pthread_mutex_t * param0, int param1, int * param2) {
    int ret_val;
    bool call_original = BeforeMutexSetprioceiling(ret_addr, param0, param1, param2);

    if (call_original)
        ret_val = Originals::pthread_mutex_setprioceiling(param0, param1, param2);
    else
        ret_val = SimulateMutexSetprioceiling(ret_addr, param0, param1, param2);

    AfterMutexSetprioceiling(ret_addr, ret_val, param0, param1, param2);
    return ret_val; 
}

int Handler::thrille_mutexattr_getprotocol(void * ret_addr, const pthread_mutexattr_t * param0, int * param1) {
    int ret_val;
    bool call_original = BeforeMutexattrGetprotocol(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_mutexattr_getprotocol(param0, param1);
    else
        ret_val = SimulateMutexattrGetprotocol(ret_addr, param0, param1);

    AfterMutexattrGetprotocol(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_attr_getstack(void * ret_addr, const pthread_attr_t * param0, void ** param1, size_t * param2) {
    int ret_val;
    bool call_original = BeforeAttrGetstack(ret_addr, param0, param1, param2);

    if (call_original)
        ret_val = Originals::pthread_attr_getstack(param0, param1, param2);
    else
        ret_val = SimulateAttrGetstack(ret_addr, param0, param1, param2);

    AfterAttrGetstack(ret_addr, ret_val, param0, param1, param2);
    return ret_val; 
}

int Handler::thrille_condattr_init(void * ret_addr, pthread_condattr_t * param0) {
    int ret_val;
    bool call_original = BeforeCondattrInit(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_condattr_init(param0);
    else
        ret_val = SimulateCondattrInit(ret_addr, param0);

    AfterCondattrInit(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_mutexattr_gettype(void * ret_addr, const pthread_mutexattr_t * param0, int * param1) {
    int ret_val;
    bool call_original = BeforeMutexattrGettype(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_mutexattr_gettype(param0, param1);
    else
        ret_val = SimulateMutexattrGettype(ret_addr, param0, param1);

    AfterMutexattrGettype(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::sem_post(void * ret_addr, sem_t * param0) {
    int ret_val;
    bool call_original = BeforeSemPost(ret_addr, param0);

    if (call_original)
        ret_val = Originals::sem_post(param0);
    else
        ret_val = SimulateSemPost(ret_addr, param0);

    AfterSemPost(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_attr_getstacksize(void * ret_addr, const pthread_attr_t * param0, size_t * param1) {
    int ret_val;
    bool call_original = BeforeAttrGetstacksize(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_getstacksize(param0, param1);
    else
        ret_val = SimulateAttrGetstacksize(ret_addr, param0, param1);

    AfterAttrGetstacksize(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::sem_timedwait(void * ret_addr, sem_t * param0, const struct timespec * param1) {
    int ret_val;
    bool call_original = BeforeSemTimedwait(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::sem_timedwait(param0, param1);
    else
        ret_val = SimulateSemTimedwait(ret_addr, param0, param1);

    AfterSemTimedwait(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_equal(void * ret_addr, pthread_t param0, pthread_t param1) {
    int ret_val;
    bool call_original = BeforeEqual(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_equal(param0, param1);
    else
        ret_val = SimulateEqual(ret_addr, param0, param1);

    AfterEqual(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_spin_init(void * ret_addr, pthread_spinlock_t * param0, int param1) {
    int ret_val;
    bool call_original = BeforeSpinInit(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_spin_init(param0, param1);
    else
        ret_val = SimulateSpinInit(ret_addr, param0, param1);

    AfterSpinInit(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::sched_yield(void * ret_addr) {
    int ret_val;
    bool call_original = BeforeSchedYield(ret_addr);

    if (call_original)
        ret_val = Originals::sched_yield();
    else
        ret_val = SimulateSchedYield(ret_addr);

    AfterSchedYield(ret_addr, ret_val);
    return ret_val; 
}

int Handler::thrille_key_create(void * ret_addr, pthread_key_t * param0, void (* param1)(void *)) {
    int ret_val;
    bool call_original = BeforeKeyCreate(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_key_create(param0, param1);
    else
        ret_val = SimulateKeyCreate(ret_addr, param0, param1);

    AfterKeyCreate(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_mutexattr_destroy(void * ret_addr, pthread_mutexattr_t * param0) {
    int ret_val;
    bool call_original = BeforeMutexattrDestroy(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_mutexattr_destroy(param0);
    else
        ret_val = SimulateMutexattrDestroy(ret_addr, param0);

    AfterMutexattrDestroy(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_cond_timedwait(void * ret_addr, pthread_cond_t * param0, pthread_mutex_t * param1, const struct timespec * param2) {
    int ret_val;
    bool call_original = BeforeCondTimedwait(ret_addr, param0, param1, param2);

    if (call_original)
        ret_val = Originals::pthread_cond_timedwait(param0, param1, param2);
    else
        ret_val = SimulateCondTimedwait(ret_addr, param0, param1, param2);

    AfterCondTimedwait(ret_addr, ret_val, param0, param1, param2);
    return ret_val; 
}

int Handler::thrille_condattr_getclock(void * ret_addr, const pthread_condattr_t * param0, clockid_t * param1) {
    int ret_val;
    bool call_original = BeforeCondattrGetclock(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_condattr_getclock(param0, param1);
    else
        ret_val = SimulateCondattrGetclock(ret_addr, param0, param1);

    AfterCondattrGetclock(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_setschedparam(void * ret_addr, pthread_t param0, int param1, const struct sched_param * param2) {
    int ret_val;
    bool call_original = BeforeSetschedparam(ret_addr, param0, param1, param2);

    if (call_original)
        ret_val = Originals::pthread_setschedparam(param0, param1, param2);
    else
        ret_val = SimulateSetschedparam(ret_addr, param0, param1, param2);

    AfterSetschedparam(ret_addr, ret_val, param0, param1, param2);
    return ret_val; 
}

int Handler::thrille_attr_setinheritsched(void * ret_addr, pthread_attr_t * param0, int param1) {
    int ret_val;
    bool call_original = BeforeAttrSetinheritsched(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_setinheritsched(param0, param1);
    else
        ret_val = SimulateAttrSetinheritsched(ret_addr, param0, param1);

    AfterAttrSetinheritsched(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_mutex_unlock(void * ret_addr, pthread_mutex_t * param0) {
    int ret_val;
    bool call_original = BeforeMutexUnlock(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_mutex_unlock(param0);
    else
        ret_val = SimulateMutexUnlock(ret_addr, param0);

    AfterMutexUnlock(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_attr_setschedparam(void * ret_addr, pthread_attr_t * param0, const struct sched_param * param1) {
    int ret_val;
    bool call_original = BeforeAttrSetschedparam(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_setschedparam(param0, param1);
    else
        ret_val = SimulateAttrSetschedparam(ret_addr, param0, param1);

    AfterAttrSetschedparam(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_rwlockattr_setpshared(void * ret_addr, pthread_rwlockattr_t * param0, int param1) {
    int ret_val;
    bool call_original = BeforeRwlockattrSetpshared(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_rwlockattr_setpshared(param0, param1);
    else
        ret_val = SimulateRwlockattrSetpshared(ret_addr, param0, param1);

    AfterRwlockattrSetpshared(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_attr_setstackaddr(void * ret_addr, pthread_attr_t * param0, void * param1) {
    int ret_val;
    bool call_original = BeforeAttrSetstackaddr(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_setstackaddr(param0, param1);
    else
        ret_val = SimulateAttrSetstackaddr(ret_addr, param0, param1);

    AfterAttrSetstackaddr(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_mutex_destroy(void * ret_addr, pthread_mutex_t * param0) {
    int ret_val;
    bool call_original = BeforeMutexDestroy(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_mutex_destroy(param0);
    else
        ret_val = SimulateMutexDestroy(ret_addr, param0);

    AfterMutexDestroy(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_attr_getschedpolicy(void * ret_addr, const pthread_attr_t * param0, int * param1) {
    int ret_val;
    bool call_original = BeforeAttrGetschedpolicy(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_getschedpolicy(param0, param1);
    else
        ret_val = SimulateAttrGetschedpolicy(ret_addr, param0, param1);

    AfterAttrGetschedpolicy(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_setcanceltype(void * ret_addr, int param0, int * param1) {
    int ret_val;
    bool call_original = BeforeSetcanceltype(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_setcanceltype(param0, param1);
    else
        ret_val = SimulateSetcanceltype(ret_addr, param0, param1);

    AfterSetcanceltype(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::sigwait(void * ret_addr, const sigset_t * param0, int * param1) {
    int ret_val;
    bool call_original = BeforeSigwait(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::sigwait(param0, param1);
    else
        ret_val = SimulateSigwait(ret_addr, param0, param1);

    AfterSigwait(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_rwlock_init(void * ret_addr, pthread_rwlock_t * param0, const pthread_rwlockattr_t * param1) {
    int ret_val;
    bool call_original = BeforeRwlockInit(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_rwlock_init(param0, param1);
    else
        ret_val = SimulateRwlockInit(ret_addr, param0, param1);

    AfterRwlockInit(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_rwlock_unlock(void * ret_addr, pthread_rwlock_t * param0) {
    int ret_val;
    bool call_original = BeforeRwlockUnlock(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_rwlock_unlock(param0);
    else
        ret_val = SimulateRwlockUnlock(ret_addr, param0);

    AfterRwlockUnlock(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_mutexattr_setprotocol(void * ret_addr, pthread_mutexattr_t * param0, int param1) {
    int ret_val;
    bool call_original = BeforeMutexattrSetprotocol(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_mutexattr_setprotocol(param0, param1);
    else
        ret_val = SimulateMutexattrSetprotocol(ret_addr, param0, param1);

    AfterMutexattrSetprotocol(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_mutex_getprioceiling(void * ret_addr, const pthread_mutex_t * param0, int * param1) {
    int ret_val;
    bool call_original = BeforeMutexGetprioceiling(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_mutex_getprioceiling(param0, param1);
    else
        ret_val = SimulateMutexGetprioceiling(ret_addr, param0, param1);

    AfterMutexGetprioceiling(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::sem_trywait(void * ret_addr, sem_t * param0) {
    int ret_val;
    bool call_original = BeforeSemTrywait(ret_addr, param0);

    if (call_original)
        ret_val = Originals::sem_trywait(param0);
    else
        ret_val = SimulateSemTrywait(ret_addr, param0);

    AfterSemTrywait(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_attr_getstackaddr(void * ret_addr, const pthread_attr_t * param0, void ** param1) {
    int ret_val;
    bool call_original = BeforeAttrGetstackaddr(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_getstackaddr(param0, param1);
    else
        ret_val = SimulateAttrGetstackaddr(ret_addr, param0, param1);

    AfterAttrGetstackaddr(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_mutexattr_settype(void * ret_addr, pthread_mutexattr_t * param0, int param1) {
    int ret_val;
    bool call_original = BeforeMutexattrSettype(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_mutexattr_settype(param0, param1);
    else
        ret_val = SimulateMutexattrSettype(ret_addr, param0, param1);

    AfterMutexattrSettype(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_getcpuclockid(void * ret_addr, pthread_t param0, clockid_t * param1) {
    int ret_val;
    bool call_original = BeforeGetcpuclockid(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_getcpuclockid(param0, param1);
    else
        ret_val = SimulateGetcpuclockid(ret_addr, param0, param1);

    AfterGetcpuclockid(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_cond_destroy(void * ret_addr, pthread_cond_t * param0) {
    int ret_val;
    bool call_original = BeforeCondDestroy(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_cond_destroy(param0);
    else
        ret_val = SimulateCondDestroy(ret_addr, param0);

    AfterCondDestroy(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_spin_lock(void * ret_addr, pthread_spinlock_t * param0) {
    int ret_val;
    bool call_original = BeforeSpinLock(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_spin_lock(param0);
    else
        ret_val = SimulateSpinLock(ret_addr, param0);

    AfterSpinLock(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_once(void * ret_addr, pthread_once_t * param0, void (* param1)(void)) {
    int ret_val;
    bool call_original = BeforeOnce(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_once(param0, param1);
    else
        ret_val = SimulateOnce(ret_addr, param0, param1);

    AfterOnce(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_atfork(void * ret_addr, void (* param0)(void), void (* param1)(void), void(* param2)(void)) {
    int ret_val;
    bool call_original = BeforeAtfork(ret_addr, param0, param1, param2);

    if (call_original)
        ret_val = Originals::pthread_atfork(param0, param1, param2);
    else
        ret_val = SimulateAtfork(ret_addr, param0, param1, param2);

    AfterAtfork(ret_addr, ret_val, param0, param1, param2);
    return ret_val; 
}

int Handler::thrille_attr_setscope(void * ret_addr, pthread_attr_t * param0, int param1) {
    int ret_val;
    bool call_original = BeforeAttrSetscope(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_setscope(param0, param1);
    else
        ret_val = SimulateAttrSetscope(ret_addr, param0, param1);

    AfterAttrSetscope(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

void * Handler::thrille_getspecific(void * ret_addr, pthread_key_t param0) {
    void * ret_val;
    bool call_original = BeforeGetspecific(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_getspecific(param0);
    else
        ret_val = SimulateGetspecific(ret_addr, param0);

    AfterGetspecific(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_attr_destroy(void * ret_addr, pthread_attr_t * param0) {
    int ret_val;
    bool call_original = BeforeAttrDestroy(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_attr_destroy(param0);
    else
        ret_val = SimulateAttrDestroy(ret_addr, param0);

    AfterAttrDestroy(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_mutex_timedlock(void * ret_addr, pthread_mutex_t * param0, const struct timespec * param1) {
    int ret_val;
    bool call_original = BeforeMutexTimedlock(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_mutex_timedlock(param0, param1);
    else
        ret_val = SimulateMutexTimedlock(ret_addr, param0, param1);

    AfterMutexTimedlock(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_barrier_init(void * ret_addr, pthread_barrier_t * param0, const pthread_barrierattr_t * param1, unsigned param2) {
    int ret_val;
    bool call_original = BeforeBarrierInit(ret_addr, param0, param1, param2);

    if (call_original)
        ret_val = Originals::pthread_barrier_init(param0, param1, param2);
    else
        ret_val = SimulateBarrierInit(ret_addr, param0, param1, param2);

    AfterBarrierInit(ret_addr, ret_val, param0, param1, param2);
    return ret_val; 
}

int Handler::thrille_setschedprio(void * ret_addr, pthread_t param0, int param1) {
    int ret_val;
    bool call_original = BeforeSetschedprio(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_setschedprio(param0, param1);
    else
        ret_val = SimulateSetschedprio(ret_addr, param0, param1);

    AfterSetschedprio(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_condattr_setclock(void * ret_addr, pthread_condattr_t * param0, clockid_t param1) {
    int ret_val;
    bool call_original = BeforeCondattrSetclock(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_condattr_setclock(param0, param1);
    else
        ret_val = SimulateCondattrSetclock(ret_addr, param0, param1);

    AfterCondattrSetclock(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_cond_broadcast(void * ret_addr, pthread_cond_t * param0) {
    int ret_val;
    bool call_original = BeforeCondBroadcast(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_cond_broadcast(param0);
    else
        ret_val = SimulateCondBroadcast(ret_addr, param0);

    AfterCondBroadcast(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_spin_unlock(void * ret_addr, pthread_spinlock_t * param0) {
    int ret_val;
    bool call_original = BeforeSpinUnlock(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_spin_unlock(param0);
    else
        ret_val = SimulateSpinUnlock(ret_addr, param0);

    AfterSpinUnlock(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_rwlock_rdlock(void * ret_addr, pthread_rwlock_t * param0) {
    int ret_val;
    bool call_original = BeforeRwlockRdlock(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_rwlock_rdlock(param0);
    else
        ret_val = SimulateRwlockRdlock(ret_addr, param0);

    AfterRwlockRdlock(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_rwlock_wrlock(void * ret_addr, pthread_rwlock_t * param0) {
    int ret_val;
    bool call_original = BeforeRwlockWrlock(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_rwlock_wrlock(param0);
    else
        ret_val = SimulateRwlockWrlock(ret_addr, param0);

    AfterRwlockWrlock(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_join(void * ret_addr, pthread_t param0, void ** param1) {
    int ret_val;
    bool call_original = BeforeJoin(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_join(param0, param1);
    else
        ret_val = SimulateJoin(ret_addr, param0, param1);

    AfterJoin(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_cond_init(void * ret_addr, pthread_cond_t * param0, const pthread_condattr_t * param1) {
    int ret_val;
    bool call_original = BeforeCondInit(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_cond_init(param0, param1);
    else
        ret_val = SimulateCondInit(ret_addr, param0, param1);

    AfterCondInit(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_attr_getguardsize(void * ret_addr, const pthread_attr_t * param0, size_t * param1) {
    int ret_val;
    bool call_original = BeforeAttrGetguardsize(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_getguardsize(param0, param1);
    else
        ret_val = SimulateAttrGetguardsize(ret_addr, param0, param1);

    AfterAttrGetguardsize(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_barrierattr_init(void * ret_addr, pthread_barrierattr_t * param0) {
    int ret_val;
    bool call_original = BeforeBarrierattrInit(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_barrierattr_init(param0);
    else
        ret_val = SimulateBarrierattrInit(ret_addr, param0);

    AfterBarrierattrInit(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_rwlock_tryrdlock(void * ret_addr, pthread_rwlock_t * param0) {
    int ret_val;
    bool call_original = BeforeRwlockTryrdlock(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_rwlock_tryrdlock(param0);
    else
        ret_val = SimulateRwlockTryrdlock(ret_addr, param0);

    AfterRwlockTryrdlock(ret_addr, ret_val, param0);
    return ret_val; 
}

int Handler::thrille_attr_setschedpolicy(void * ret_addr, pthread_attr_t * param0, int param1) {
    int ret_val;
    bool call_original = BeforeAttrSetschedpolicy(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_setschedpolicy(param0, param1);
    else
        ret_val = SimulateAttrSetschedpolicy(ret_addr, param0, param1);

    AfterAttrSetschedpolicy(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_barrierattr_destroy(void * ret_addr, pthread_barrierattr_t * param0) {
    int ret_val;
    bool call_original = BeforeBarrierattrDestroy(ret_addr, param0);

    if (call_original)
        ret_val = Originals::pthread_barrierattr_destroy(param0);
    else
        ret_val = SimulateBarrierattrDestroy(ret_addr, param0);

    AfterBarrierattrDestroy(ret_addr, ret_val, param0);
    return ret_val; 
}

pthread_t Handler::thrille_self(void * ret_addr) {
    pthread_t ret_val;
    bool call_original = BeforeSelf(ret_addr);

    if (call_original)
        ret_val = Originals::pthread_self();
    else
        ret_val = SimulateSelf(ret_addr);

    AfterSelf(ret_addr, ret_val);
    return ret_val; 
}

int Handler::thrille_attr_getdetachstate(void * ret_addr, const pthread_attr_t * param0, int * param1) {
    int ret_val;
    bool call_original = BeforeAttrGetdetachstate(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_attr_getdetachstate(param0, param1);
    else
        ret_val = SimulateAttrGetdetachstate(ret_addr, param0, param1);

    AfterAttrGetdetachstate(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_rwlock_timedwrlock(void * ret_addr, pthread_rwlock_t * param0, const struct timespec * param1) {
    int ret_val;
    bool call_original = BeforeRwlockTimedwrlock(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_rwlock_timedwrlock(param0, param1);
    else
        ret_val = SimulateRwlockTimedwrlock(ret_addr, param0, param1);

    AfterRwlockTimedwrlock(ret_addr, ret_val, param0, param1);
    return ret_val; 
}

int Handler::thrille_mutexattr_setprioceiling(void * ret_addr, pthread_mutexattr_t * param0, int param1) {
    int ret_val;
    bool call_original = BeforeMutexattrSetprioceiling(ret_addr, param0, param1);

    if (call_original)
        ret_val = Originals::pthread_mutexattr_setprioceiling(param0, param1);
    else
        ret_val = SimulateMutexattrSetprioceiling(ret_addr, param0, param1);

    AfterMutexattrSetprioceiling(ret_addr, ret_val, param0, param1);
    return ret_val; 
}


//Implementation of empty Interposition hooks

bool Handler::BeforeCreate(void * ret_addr,
        pthread_t * param0, 
        const pthread_attr_t * param1,
        void *(* param2)(void *),
        void * param3,
        thrID & param4) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true; 
}

int Handler::SimulateCreate(void * ret_addr,
        pthread_t * param0,
        const pthread_attr_t * param1,
        void *(* param2)(void *),
        void * param3,
        thrID & param4) {return 0;}


void Handler::AfterCreate(void * ret_addr,
        int ret_val,
        pthread_t * param0,
        const pthread_attr_t * param1,
        void *(* param2)(void *),
        void * param3,
        thrID & param4) { }

bool Handler::BeforeSemWait(void * ret_addr, sem_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSemWait(void * ret_addr, sem_t * param0) {return 0;}
void Handler::AfterSemWait(void * ret_addr, int ret_val, sem_t * param0) { }

bool Handler::BeforeMutexLock(void * ret_addr, pthread_mutex_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexLock(void * ret_addr, pthread_mutex_t * param0) {return 0;}
void Handler::AfterMutexLock(void * ret_addr, int ret_val, pthread_mutex_t * param0) { }

bool Handler::BeforeSleep(void * ret_addr, unsigned int param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
unsigned int Handler::SimulateSleep(void * ret_addr, unsigned int param0) {return 0;}
void Handler::AfterSleep(void * ret_addr, unsigned int ret_val, unsigned int param0) { }

bool Handler::BeforeCondattrDestroy(void * ret_addr, pthread_condattr_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCondattrDestroy(void * ret_addr, pthread_condattr_t * param0) {return 0;}
void Handler::AfterCondattrDestroy(void * ret_addr, int ret_val, pthread_condattr_t * param0) { }

bool Handler::BeforeMutexattrGetpshared(void * ret_addr, const pthread_mutexattr_t * param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexattrGetpshared(void * ret_addr, const pthread_mutexattr_t * param0, int * param1) {return 0;}
void Handler::AfterMutexattrGetpshared(void * ret_addr, int ret_val, const pthread_mutexattr_t * param0, int * param1) { }

bool Handler::BeforeSetconcurrency(void * ret_addr, int param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSetconcurrency(void * ret_addr, int param0) {return 0;}
void Handler::AfterSetconcurrency(void * ret_addr, int ret_val, int param0) { }

bool Handler::BeforeGetconcurrency(void * ret_addr) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateGetconcurrency(void * ret_addr) {return 0;}
void Handler::AfterGetconcurrency(void * ret_addr, int ret_val) { }

bool Handler::BeforeSemDestroy(void * ret_addr, sem_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSemDestroy(void * ret_addr, sem_t * param0) {return 0;}
void Handler::AfterSemDestroy(void * ret_addr, int ret_val, sem_t * param0) { }

bool Handler::BeforeTestcancel(void * ret_addr) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
void Handler::SimulateTestcancel(void * ret_addr) {return;}
void Handler::AfterTestcancel(void * ret_addr) { }

bool Handler::BeforeAttrGetscope(void * ret_addr, const pthread_attr_t * param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrGetscope(void * ret_addr, const pthread_attr_t * param0, int * param1) {return 0;}
void Handler::AfterAttrGetscope(void * ret_addr, int ret_val, const pthread_attr_t * param0, int * param1) { }

bool Handler::BeforeMutexInit(void * ret_addr, pthread_mutex_t * param0, const pthread_mutexattr_t * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexInit(void * ret_addr, pthread_mutex_t * param0, const pthread_mutexattr_t * param1) {return 0;}
void Handler::AfterMutexInit(void * ret_addr, int ret_val, pthread_mutex_t * param0, const pthread_mutexattr_t * param1) { }

bool Handler::BeforeSemInit(void * ret_addr, sem_t * param0, int param1, unsigned int param2) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSemInit(void * ret_addr, sem_t * param0, int param1, unsigned int param2) {return 0;}
void Handler::AfterSemInit(void * ret_addr, int ret_val, sem_t * param0, int param1, unsigned int param2) { }

bool Handler::BeforeMutexTrylock(void * ret_addr, pthread_mutex_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexTrylock(void * ret_addr, pthread_mutex_t * param0) {return 0;}
void Handler::AfterMutexTrylock(void * ret_addr, int ret_val, pthread_mutex_t * param0) { }

bool Handler::BeforeRwlockattrInit(void * ret_addr, pthread_rwlockattr_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateRwlockattrInit(void * ret_addr, pthread_rwlockattr_t * param0) {return 0;}
void Handler::AfterRwlockattrInit(void * ret_addr, int ret_val, pthread_rwlockattr_t * param0) { }

bool Handler::BeforeMutexattrSetpshared(void * ret_addr, pthread_mutexattr_t * param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexattrSetpshared(void * ret_addr, pthread_mutexattr_t * param0, int param1) {return 0;}
void Handler::AfterMutexattrSetpshared(void * ret_addr, int ret_val, pthread_mutexattr_t * param0, int param1) { }

bool Handler::BeforeRwlockattrDestroy(void * ret_addr, pthread_rwlockattr_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateRwlockattrDestroy(void * ret_addr, pthread_rwlockattr_t * param0) {return 0;}
void Handler::AfterRwlockattrDestroy(void * ret_addr, int ret_val, pthread_rwlockattr_t * param0) { }

bool Handler::BeforeBarrierDestroy(void * ret_addr, pthread_barrier_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateBarrierDestroy(void * ret_addr, pthread_barrier_t * param0) {return 0;}
void Handler::AfterBarrierDestroy(void * ret_addr, int ret_val, pthread_barrier_t * param0) { }

bool Handler::BeforeGetschedparam(void * ret_addr, pthread_t param0, int * param1, struct sched_param * param2) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateGetschedparam(void * ret_addr, pthread_t param0, int * param1, struct sched_param * param2) {return 0;}
void Handler::AfterGetschedparam(void * ret_addr, int ret_val, pthread_t param0, int * param1, struct sched_param * param2) { }

bool Handler::BeforeAttrGetschedparam(void * ret_addr, const pthread_attr_t * param0, struct sched_param * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrGetschedparam(void * ret_addr, const pthread_attr_t * param0, struct sched_param * param1) {return 0;}
void Handler::AfterAttrGetschedparam(void * ret_addr, int ret_val, const pthread_attr_t * param0, struct sched_param * param1) { }

bool Handler::BeforeCancel(void * ret_addr, pthread_t param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCancel(void * ret_addr, pthread_t param0) {return 0;}
void Handler::AfterCancel(void * ret_addr, int ret_val, pthread_t param0) { }

bool Handler::BeforeCondWait(void * ret_addr, pthread_cond_t * param0, pthread_mutex_t * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCondWait(void * ret_addr, pthread_cond_t * param0, pthread_mutex_t * param1) {return 0;}
void Handler::AfterCondWait(void * ret_addr, int ret_val, pthread_cond_t * param0, pthread_mutex_t * param1) { }

bool Handler::BeforeCondattrGetpshared(void * ret_addr, const pthread_condattr_t * param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCondattrGetpshared(void * ret_addr, const pthread_condattr_t * param0, int * param1) {return 0;}
void Handler::AfterCondattrGetpshared(void * ret_addr, int ret_val, const pthread_condattr_t * param0, int * param1) { }

bool Handler::BeforeSpinTrylock(void * ret_addr, pthread_spinlock_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSpinTrylock(void * ret_addr, pthread_spinlock_t * param0) {return 0;}
void Handler::AfterSpinTrylock(void * ret_addr, int ret_val, pthread_spinlock_t * param0) { }

bool Handler::BeforeAttrGetinheritsched(void * ret_addr, const pthread_attr_t * param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrGetinheritsched(void * ret_addr, const pthread_attr_t * param0, int * param1) {return 0;}
void Handler::AfterAttrGetinheritsched(void * ret_addr, int ret_val, const pthread_attr_t * param0, int * param1) { }

bool Handler::BeforeAttrSetstack(void * ret_addr, pthread_attr_t * param0, void * param1, size_t param2) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrSetstack(void * ret_addr, pthread_attr_t * param0, void * param1, size_t param2) {return 0;}
void Handler::AfterAttrSetstack(void * ret_addr, int ret_val, pthread_attr_t * param0, void * param1, size_t param2) { }

bool Handler::BeforeCondSignal(void * ret_addr, pthread_cond_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCondSignal(void * ret_addr, pthread_cond_t * param0) {return 0;}
void Handler::AfterCondSignal(void * ret_addr, int ret_val, pthread_cond_t * param0) { }

bool Handler::BeforeSemGetvalue(void * ret_addr, sem_t * param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSemGetvalue(void * ret_addr, sem_t * param0, int * param1) {return 0;}
void Handler::AfterSemGetvalue(void * ret_addr, int ret_val, sem_t * param0, int * param1) { }

bool Handler::BeforeMutexattrInit(void * ret_addr, pthread_mutexattr_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexattrInit(void * ret_addr, pthread_mutexattr_t * param0) {return 0;}
void Handler::AfterMutexattrInit(void * ret_addr, int ret_val, pthread_mutexattr_t * param0) { }

bool Handler::BeforeBarrierWait(void * ret_addr, pthread_barrier_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateBarrierWait(void * ret_addr, pthread_barrier_t * param0) {return 0;}
void Handler::AfterBarrierWait(void * ret_addr, int ret_val, pthread_barrier_t * param0) { }

bool Handler::BeforeSpinDestroy(void * ret_addr, pthread_spinlock_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSpinDestroy(void * ret_addr, pthread_spinlock_t * param0) {return 0;}
void Handler::AfterSpinDestroy(void * ret_addr, int ret_val, pthread_spinlock_t * param0) { }

bool Handler::BeforeKeyDelete(void * ret_addr, pthread_key_t param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateKeyDelete(void * ret_addr, pthread_key_t param0) {return 0;}
void Handler::AfterKeyDelete(void * ret_addr, int ret_val, pthread_key_t param0) { }

bool Handler::BeforeSetspecific(void * ret_addr, pthread_key_t param0, const void * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSetspecific(void * ret_addr, pthread_key_t param0, const void * param1) {return 0;}
void Handler::AfterSetspecific(void * ret_addr, int ret_val, pthread_key_t param0, const void * param1) { }

bool Handler::BeforeAttrSetstacksize(void * ret_addr, pthread_attr_t * param0, size_t param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrSetstacksize(void * ret_addr, pthread_attr_t * param0, size_t param1) {return 0;}
void Handler::AfterAttrSetstacksize(void * ret_addr, int ret_val, pthread_attr_t * param0, size_t param1) { }

bool Handler::BeforeSetcancelstate(void * ret_addr, int param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSetcancelstate(void * ret_addr, int param0, int * param1) {return 0;}
void Handler::AfterSetcancelstate(void * ret_addr, int ret_val, int param0, int * param1) { }

bool Handler::BeforeBarrierattrSetpshared(void * ret_addr, pthread_barrierattr_t * param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateBarrierattrSetpshared(void * ret_addr, pthread_barrierattr_t * param0, int param1) {return 0;}
void Handler::AfterBarrierattrSetpshared(void * ret_addr, int ret_val, pthread_barrierattr_t * param0, int param1) { }

bool Handler::BeforeRwlockTimedrdlock(void * ret_addr, pthread_rwlock_t * param0, const struct timespec * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateRwlockTimedrdlock(void * ret_addr, pthread_rwlock_t * param0, const struct timespec * param1) {return 0;}
void Handler::AfterRwlockTimedrdlock(void * ret_addr, int ret_val, pthread_rwlock_t * param0, const struct timespec * param1) { }

bool Handler::BeforeDetach(void * ret_addr, pthread_t param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateDetach(void * ret_addr, pthread_t param0) {return 0;}
void Handler::AfterDetach(void * ret_addr, int ret_val, pthread_t param0) { }

bool Handler::BeforeUsleep(void * ret_addr, useconds_t param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateUsleep(void * ret_addr, useconds_t param0) {return 0;}
void Handler::AfterUsleep(void * ret_addr, int ret_val, useconds_t param0) { }

bool Handler::BeforeRwlockDestroy(void * ret_addr, pthread_rwlock_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateRwlockDestroy(void * ret_addr, pthread_rwlock_t * param0) {return 0;}
void Handler::AfterRwlockDestroy(void * ret_addr, int ret_val, pthread_rwlock_t * param0) { }

bool Handler::BeforeExit(void * ret_addr, void * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
void Handler::SimulateExit(void * ret_addr, void * param0) {return;}
void Handler::AfterExit(void * ret_addr, void * param0) { }

bool Handler::BeforeAttrSetdetachstate(void * ret_addr, pthread_attr_t * param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrSetdetachstate(void * ret_addr, pthread_attr_t * param0, int param1) {return 0;}
void Handler::AfterAttrSetdetachstate(void * ret_addr, int ret_val, pthread_attr_t * param0, int param1) { }

bool Handler::BeforeRwlockTrywrlock(void * ret_addr, pthread_rwlock_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateRwlockTrywrlock(void * ret_addr, pthread_rwlock_t * param0) {return 0;}
void Handler::AfterRwlockTrywrlock(void * ret_addr, int ret_val, pthread_rwlock_t * param0) { }

bool Handler::BeforeAttrInit(void * ret_addr, pthread_attr_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrInit(void * ret_addr, pthread_attr_t * param0) {return 0;}
void Handler::AfterAttrInit(void * ret_addr, int ret_val, pthread_attr_t * param0) { }

bool Handler::BeforeAttrSetguardsize(void * ret_addr, pthread_attr_t * param0, size_t param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrSetguardsize(void * ret_addr, pthread_attr_t * param0, size_t param1) {return 0;}
void Handler::AfterAttrSetguardsize(void * ret_addr, int ret_val, pthread_attr_t * param0, size_t param1) { }

bool Handler::BeforeCondattrSetpshared(void * ret_addr, pthread_condattr_t * param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCondattrSetpshared(void * ret_addr, pthread_condattr_t * param0, int param1) {return 0;}
void Handler::AfterCondattrSetpshared(void * ret_addr, int ret_val, pthread_condattr_t * param0, int param1) { }

bool Handler::BeforeMutexSetprioceiling(void * ret_addr, pthread_mutex_t * param0, int param1, int * param2) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexSetprioceiling(void * ret_addr, pthread_mutex_t * param0, int param1, int * param2) {return 0;}
void Handler::AfterMutexSetprioceiling(void * ret_addr, int ret_val, pthread_mutex_t * param0, int param1, int * param2) { }

bool Handler::BeforeMutexattrGetprotocol(void * ret_addr, const pthread_mutexattr_t * param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexattrGetprotocol(void * ret_addr, const pthread_mutexattr_t * param0, int * param1) {return 0;}
void Handler::AfterMutexattrGetprotocol(void * ret_addr, int ret_val, const pthread_mutexattr_t * param0, int * param1) { }

bool Handler::BeforeAttrGetstack(void * ret_addr, const pthread_attr_t * param0, void ** param1, size_t * param2) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrGetstack(void * ret_addr, const pthread_attr_t * param0, void ** param1, size_t * param2) {return 0;}
void Handler::AfterAttrGetstack(void * ret_addr, int ret_val, const pthread_attr_t * param0, void ** param1, size_t * param2) { }

bool Handler::BeforeCondattrInit(void * ret_addr, pthread_condattr_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCondattrInit(void * ret_addr, pthread_condattr_t * param0) {return 0;}
void Handler::AfterCondattrInit(void * ret_addr, int ret_val, pthread_condattr_t * param0) { }

bool Handler::BeforeMutexattrGettype(void * ret_addr, const pthread_mutexattr_t * param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexattrGettype(void * ret_addr, const pthread_mutexattr_t * param0, int * param1) {return 0;}
void Handler::AfterMutexattrGettype(void * ret_addr, int ret_val, const pthread_mutexattr_t * param0, int * param1) { }

bool Handler::BeforeSemPost(void * ret_addr, sem_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSemPost(void * ret_addr, sem_t * param0) {return 0;}
void Handler::AfterSemPost(void * ret_addr, int ret_val, sem_t * param0) { }

bool Handler::BeforeAttrGetstacksize(void * ret_addr, const pthread_attr_t * param0, size_t * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrGetstacksize(void * ret_addr, const pthread_attr_t * param0, size_t * param1) {return 0;}
void Handler::AfterAttrGetstacksize(void * ret_addr, int ret_val, const pthread_attr_t * param0, size_t * param1) { }

bool Handler::BeforeSemTimedwait(void * ret_addr, sem_t * param0, const struct timespec * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSemTimedwait(void * ret_addr, sem_t * param0, const struct timespec * param1) {return 0;}
void Handler::AfterSemTimedwait(void * ret_addr, int ret_val, sem_t * param0, const struct timespec * param1) { }

bool Handler::BeforeEqual(void * ret_addr, pthread_t param0, pthread_t param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateEqual(void * ret_addr, pthread_t param0, pthread_t param1) {return 0;}
void Handler::AfterEqual(void * ret_addr, int ret_val, pthread_t param0, pthread_t param1) { }

bool Handler::BeforeSpinInit(void * ret_addr, pthread_spinlock_t * param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSpinInit(void * ret_addr, pthread_spinlock_t * param0, int param1) {return 0;}
void Handler::AfterSpinInit(void * ret_addr, int ret_val, pthread_spinlock_t * param0, int param1) { }

bool Handler::BeforeSchedYield(void * ret_addr) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSchedYield(void * ret_addr) {return 0;}
void Handler::AfterSchedYield(void * ret_addr, int ret_val) { }

bool Handler::BeforeKeyCreate(void * ret_addr, pthread_key_t * param0, void (* param1)(void *)) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateKeyCreate(void * ret_addr, pthread_key_t * param0, void (* param1)(void *)) {return 0;}
void Handler::AfterKeyCreate(void * ret_addr, int ret_val, pthread_key_t * param0, void (* param1)(void *)) { }

bool Handler::BeforeMutexattrDestroy(void * ret_addr, pthread_mutexattr_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexattrDestroy(void * ret_addr, pthread_mutexattr_t * param0) {return 0;}
void Handler::AfterMutexattrDestroy(void * ret_addr, int ret_val, pthread_mutexattr_t * param0) { }

bool Handler::BeforeCondTimedwait(void * ret_addr, pthread_cond_t * param0, pthread_mutex_t * param1, const struct timespec * param2) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCondTimedwait(void * ret_addr, pthread_cond_t * param0, pthread_mutex_t * param1, const struct timespec * param2) {return 0;}
void Handler::AfterCondTimedwait(void * ret_addr, int ret_val, pthread_cond_t * param0, pthread_mutex_t * param1, const struct timespec * param2) { }

bool Handler::BeforeCondattrGetclock(void * ret_addr, const pthread_condattr_t * param0, clockid_t * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCondattrGetclock(void * ret_addr, const pthread_condattr_t * param0, clockid_t * param1) {return 0;}
void Handler::AfterCondattrGetclock(void * ret_addr, int ret_val, const pthread_condattr_t * param0, clockid_t * param1) { }

bool Handler::BeforeSetschedparam(void * ret_addr, pthread_t param0, int param1, const struct sched_param * param2) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSetschedparam(void * ret_addr, pthread_t param0, int param1, const struct sched_param * param2) {return 0;}
void Handler::AfterSetschedparam(void * ret_addr, int ret_val, pthread_t param0, int param1, const struct sched_param * param2) { }

bool Handler::BeforeAttrSetinheritsched(void * ret_addr, pthread_attr_t * param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrSetinheritsched(void * ret_addr, pthread_attr_t * param0, int param1) {return 0;}
void Handler::AfterAttrSetinheritsched(void * ret_addr, int ret_val, pthread_attr_t * param0, int param1) { }

bool Handler::BeforeMutexUnlock(void * ret_addr, pthread_mutex_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexUnlock(void * ret_addr, pthread_mutex_t * param0) {return 0;}
void Handler::AfterMutexUnlock(void * ret_addr, int ret_val, pthread_mutex_t * param0) { }

bool Handler::BeforeAttrSetschedparam(void * ret_addr, pthread_attr_t * param0, const struct sched_param * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrSetschedparam(void * ret_addr, pthread_attr_t * param0, const struct sched_param * param1) {return 0;}
void Handler::AfterAttrSetschedparam(void * ret_addr, int ret_val, pthread_attr_t * param0, const struct sched_param * param1) { }

bool Handler::BeforeRwlockattrSetpshared(void * ret_addr, pthread_rwlockattr_t * param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateRwlockattrSetpshared(void * ret_addr, pthread_rwlockattr_t * param0, int param1) {return 0;}
void Handler::AfterRwlockattrSetpshared(void * ret_addr, int ret_val, pthread_rwlockattr_t * param0, int param1) { }

bool Handler::BeforeAttrSetstackaddr(void * ret_addr, pthread_attr_t * param0, void * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrSetstackaddr(void * ret_addr, pthread_attr_t * param0, void * param1) {return 0;}
void Handler::AfterAttrSetstackaddr(void * ret_addr, int ret_val, pthread_attr_t * param0, void * param1) { }

bool Handler::BeforeMutexDestroy(void * ret_addr, pthread_mutex_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexDestroy(void * ret_addr, pthread_mutex_t * param0) {return 0;}
void Handler::AfterMutexDestroy(void * ret_addr, int ret_val, pthread_mutex_t * param0) { }

bool Handler::BeforeAttrGetschedpolicy(void * ret_addr, const pthread_attr_t * param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrGetschedpolicy(void * ret_addr, const pthread_attr_t * param0, int * param1) {return 0;}
void Handler::AfterAttrGetschedpolicy(void * ret_addr, int ret_val, const pthread_attr_t * param0, int * param1) { }

bool Handler::BeforeSetcanceltype(void * ret_addr, int param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSetcanceltype(void * ret_addr, int param0, int * param1) {return 0;}
void Handler::AfterSetcanceltype(void * ret_addr, int ret_val, int param0, int * param1) { }

bool Handler::BeforeSigwait(void * ret_addr, const sigset_t * param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSigwait(void * ret_addr, const sigset_t * param0, int * param1) {return 0;}
void Handler::AfterSigwait(void * ret_addr, int ret_val, const sigset_t * param0, int * param1) { }

bool Handler::BeforeRwlockInit(void * ret_addr, pthread_rwlock_t * param0, const pthread_rwlockattr_t * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateRwlockInit(void * ret_addr, pthread_rwlock_t * param0, const pthread_rwlockattr_t * param1) {return 0;}
void Handler::AfterRwlockInit(void * ret_addr, int ret_val, pthread_rwlock_t * param0, const pthread_rwlockattr_t * param1) { }

bool Handler::BeforeRwlockUnlock(void * ret_addr, pthread_rwlock_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateRwlockUnlock(void * ret_addr, pthread_rwlock_t * param0) {return 0;}
void Handler::AfterRwlockUnlock(void * ret_addr, int ret_val, pthread_rwlock_t * param0) { }

bool Handler::BeforeMutexattrSetprotocol(void * ret_addr, pthread_mutexattr_t * param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexattrSetprotocol(void * ret_addr, pthread_mutexattr_t * param0, int param1) {return 0;}
void Handler::AfterMutexattrSetprotocol(void * ret_addr, int ret_val, pthread_mutexattr_t * param0, int param1) { }

bool Handler::BeforeMutexGetprioceiling(void * ret_addr, const pthread_mutex_t * param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexGetprioceiling(void * ret_addr, const pthread_mutex_t * param0, int * param1) {return 0;}
void Handler::AfterMutexGetprioceiling(void * ret_addr, int ret_val, const pthread_mutex_t * param0, int * param1) { }

bool Handler::BeforeSemTrywait(void * ret_addr, sem_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSemTrywait(void * ret_addr, sem_t * param0) {return 0;}
void Handler::AfterSemTrywait(void * ret_addr, int ret_val, sem_t * param0) { }

bool Handler::BeforeAttrGetstackaddr(void * ret_addr, const pthread_attr_t * param0, void ** param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrGetstackaddr(void * ret_addr, const pthread_attr_t * param0, void ** param1) {return 0;}
void Handler::AfterAttrGetstackaddr(void * ret_addr, int ret_val, const pthread_attr_t * param0, void ** param1) { }

bool Handler::BeforeMutexattrSettype(void * ret_addr, pthread_mutexattr_t * param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexattrSettype(void * ret_addr, pthread_mutexattr_t * param0, int param1) {return 0;}
void Handler::AfterMutexattrSettype(void * ret_addr, int ret_val, pthread_mutexattr_t * param0, int param1) { }

bool Handler::BeforeGetcpuclockid(void * ret_addr, pthread_t param0, clockid_t * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateGetcpuclockid(void * ret_addr, pthread_t param0, clockid_t * param1) {return 0;}
void Handler::AfterGetcpuclockid(void * ret_addr, int ret_val, pthread_t param0, clockid_t * param1) { }

bool Handler::BeforeCondDestroy(void * ret_addr, pthread_cond_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCondDestroy(void * ret_addr, pthread_cond_t * param0) {return 0;}
void Handler::AfterCondDestroy(void * ret_addr, int ret_val, pthread_cond_t * param0) { }

bool Handler::BeforeSpinLock(void * ret_addr, pthread_spinlock_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSpinLock(void * ret_addr, pthread_spinlock_t * param0) {return 0;}
void Handler::AfterSpinLock(void * ret_addr, int ret_val, pthread_spinlock_t * param0) { }

bool Handler::BeforeOnce(void * ret_addr, pthread_once_t * param0, void (* param1)(void)) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateOnce(void * ret_addr, pthread_once_t * param0, void (* param1)(void)) {return 0;}
void Handler::AfterOnce(void * ret_addr, int ret_val, pthread_once_t * param0, void (* param1)(void)) { }

bool Handler::BeforeAtfork(void * ret_addr, void (* param0)(void), void (* param1)(void), void(* param2)(void)) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAtfork(void * ret_addr, void (* param0)(void), void (* param1)(void), void(* param2)(void)) {return 0;}
void Handler::AfterAtfork(void * ret_addr, int ret_val, void (* param0)(void), void (* param1)(void), void(* param2)(void)) { }

bool Handler::BeforeAttrSetscope(void * ret_addr, pthread_attr_t * param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrSetscope(void * ret_addr, pthread_attr_t * param0, int param1) {return 0;}
void Handler::AfterAttrSetscope(void * ret_addr, int ret_val, pthread_attr_t * param0, int param1) { }

bool Handler::BeforeGetspecific(void * ret_addr, pthread_key_t param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
void * Handler::SimulateGetspecific(void * ret_addr, pthread_key_t param0) {return (void *) 0;}
void Handler::AfterGetspecific(void * ret_addr, void * ret_val, pthread_key_t param0) { }

bool Handler::BeforeAttrDestroy(void * ret_addr, pthread_attr_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrDestroy(void * ret_addr, pthread_attr_t * param0) {return 0;}
void Handler::AfterAttrDestroy(void * ret_addr, int ret_val, pthread_attr_t * param0) { }

bool Handler::BeforeMutexTimedlock(void * ret_addr, pthread_mutex_t * param0, const struct timespec * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexTimedlock(void * ret_addr, pthread_mutex_t * param0, const struct timespec * param1) {return 0;}
void Handler::AfterMutexTimedlock(void * ret_addr, int ret_val, pthread_mutex_t * param0, const struct timespec * param1) { }

bool Handler::BeforeBarrierInit(void * ret_addr, pthread_barrier_t * param0, const pthread_barrierattr_t * param1, unsigned param2) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateBarrierInit(void * ret_addr, pthread_barrier_t * param0, const pthread_barrierattr_t * param1, unsigned param2) {return 0;}
void Handler::AfterBarrierInit(void * ret_addr, int ret_val, pthread_barrier_t * param0, const pthread_barrierattr_t * param1, unsigned param2) { }

bool Handler::BeforeSetschedprio(void * ret_addr, pthread_t param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSetschedprio(void * ret_addr, pthread_t param0, int param1) {return 0;}
void Handler::AfterSetschedprio(void * ret_addr, int ret_val, pthread_t param0, int param1) { }

bool Handler::BeforeCondattrSetclock(void * ret_addr, pthread_condattr_t * param0, clockid_t param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCondattrSetclock(void * ret_addr, pthread_condattr_t * param0, clockid_t param1) {return 0;}
void Handler::AfterCondattrSetclock(void * ret_addr, int ret_val, pthread_condattr_t * param0, clockid_t param1) { }

bool Handler::BeforeCondBroadcast(void * ret_addr, pthread_cond_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCondBroadcast(void * ret_addr, pthread_cond_t * param0) {return 0;}
void Handler::AfterCondBroadcast(void * ret_addr, int ret_val, pthread_cond_t * param0) { }

bool Handler::BeforeSpinUnlock(void * ret_addr, pthread_spinlock_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateSpinUnlock(void * ret_addr, pthread_spinlock_t * param0) {return 0;}
void Handler::AfterSpinUnlock(void * ret_addr, int ret_val, pthread_spinlock_t * param0) { }

bool Handler::BeforeRwlockRdlock(void * ret_addr, pthread_rwlock_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateRwlockRdlock(void * ret_addr, pthread_rwlock_t * param0) {return 0;}
void Handler::AfterRwlockRdlock(void * ret_addr, int ret_val, pthread_rwlock_t * param0) { }

bool Handler::BeforeRwlockWrlock(void * ret_addr, pthread_rwlock_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateRwlockWrlock(void * ret_addr, pthread_rwlock_t * param0) {return 0;}
void Handler::AfterRwlockWrlock(void * ret_addr, int ret_val, pthread_rwlock_t * param0) { }

bool Handler::BeforeJoin(void * ret_addr, pthread_t param0, void ** param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateJoin(void * ret_addr, pthread_t param0, void ** param1) {return 0;}
void Handler::AfterJoin(void * ret_addr, int ret_val, pthread_t param0, void ** param1) { }

bool Handler::BeforeCondInit(void * ret_addr, pthread_cond_t * param0, const pthread_condattr_t * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateCondInit(void * ret_addr, pthread_cond_t * param0, const pthread_condattr_t * param1) {return 0;}
void Handler::AfterCondInit(void * ret_addr, int ret_val, pthread_cond_t * param0, const pthread_condattr_t * param1) { }

bool Handler::BeforeAttrGetguardsize(void * ret_addr, const pthread_attr_t * param0, size_t * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrGetguardsize(void * ret_addr, const pthread_attr_t * param0, size_t * param1) {return 0;}
void Handler::AfterAttrGetguardsize(void * ret_addr, int ret_val, const pthread_attr_t * param0, size_t * param1) { }

bool Handler::BeforeBarrierattrInit(void * ret_addr, pthread_barrierattr_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateBarrierattrInit(void * ret_addr, pthread_barrierattr_t * param0) {return 0;}
void Handler::AfterBarrierattrInit(void * ret_addr, int ret_val, pthread_barrierattr_t * param0) { }

bool Handler::BeforeRwlockTryrdlock(void * ret_addr, pthread_rwlock_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateRwlockTryrdlock(void * ret_addr, pthread_rwlock_t * param0) {return 0;}
void Handler::AfterRwlockTryrdlock(void * ret_addr, int ret_val, pthread_rwlock_t * param0) { }

bool Handler::BeforeAttrSetschedpolicy(void * ret_addr, pthread_attr_t * param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrSetschedpolicy(void * ret_addr, pthread_attr_t * param0, int param1) {return 0;}
void Handler::AfterAttrSetschedpolicy(void * ret_addr, int ret_val, pthread_attr_t * param0, int param1) { }

bool Handler::BeforeBarrierattrDestroy(void * ret_addr, pthread_barrierattr_t * param0) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateBarrierattrDestroy(void * ret_addr, pthread_barrierattr_t * param0) {return 0;}
void Handler::AfterBarrierattrDestroy(void * ret_addr, int ret_val, pthread_barrierattr_t * param0) { }

bool Handler::BeforeSelf(void * ret_addr) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
pthread_t Handler::SimulateSelf(void * ret_addr) {pthread_t * s = new pthread_t;
 return *s;}
void Handler::AfterSelf(void * ret_addr, pthread_t ret_val) { }

bool Handler::BeforeAttrGetdetachstate(void * ret_addr, const pthread_attr_t * param0, int * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateAttrGetdetachstate(void * ret_addr, const pthread_attr_t * param0, int * param1) {return 0;}
void Handler::AfterAttrGetdetachstate(void * ret_addr, int ret_val, const pthread_attr_t * param0, int * param1) { }

bool Handler::BeforeRwlockTimedwrlock(void * ret_addr, pthread_rwlock_t * param0, const struct timespec * param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateRwlockTimedwrlock(void * ret_addr, pthread_rwlock_t * param0, const struct timespec * param1) {return 0;}
void Handler::AfterRwlockTimedwrlock(void * ret_addr, int ret_val, pthread_rwlock_t * param0, const struct timespec * param1) { }

bool Handler::BeforeMutexattrSetprioceiling(void * ret_addr, pthread_mutexattr_t * param0, int param1) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    return true;
}
int Handler::SimulateMutexattrSetprioceiling(void * ret_addr, pthread_mutexattr_t * param0, int param1) {return 0;}
void Handler::AfterMutexattrSetprioceiling(void * ret_addr, int ret_val, pthread_mutexattr_t * param0, int param1) { }


//Implementation of Thrille Interposition methods

int pthread_create (pthread_t* thr,
        const pthread_attr_t *attr, 
        void *(*start_routine) (void *), 
        void *arg) {
    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_create(ret_addr, thr, attr, start_routine, arg);
}

int sem_wait(sem_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("sem_wait is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("sem_wait initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->sem_wait(ret_addr, param0);
}

int pthread_mutex_lock(pthread_mutex_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutex_lock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutex_lock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutex_lock(ret_addr, param0);
}

unsigned int sleep(unsigned int param0) {
    if (! Handler::pth_is_initialized) {
        printf("sleep is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("sleep initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->sleep(ret_addr, param0);
}

int pthread_condattr_destroy(pthread_condattr_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_condattr_destroy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_condattr_destroy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_condattr_destroy(ret_addr, param0);
}

int pthread_mutexattr_getpshared(const pthread_mutexattr_t * param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutexattr_getpshared is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutexattr_getpshared initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutexattr_getpshared(ret_addr, param0, param1);
}

int pthread_setconcurrency(int param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_setconcurrency is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_setconcurrency initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_setconcurrency(ret_addr, param0);
}

int pthread_getconcurrency() __THROW {
    if (! Handler::pth_is_initialized) {
        printf("pthread_getconcurrency is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_getconcurrency initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_getconcurrency(ret_addr);
}

int sem_destroy(sem_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("sem_destroy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("sem_destroy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->sem_destroy(ret_addr, param0);
}

void pthread_testcancel() {
    if (! Handler::pth_is_initialized) {
        printf("pthread_testcancel is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_testcancel initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_testcancel(ret_addr);
}

int pthread_attr_getscope(const pthread_attr_t * param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_getscope is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_getscope initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_getscope(ret_addr, param0, param1);
}

int pthread_mutex_init(pthread_mutex_t * param0, const pthread_mutexattr_t * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutex_init is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutex_init initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutex_init(ret_addr, param0, param1);
}

int sem_init(sem_t * param0, int param1, unsigned int param2) {
    if (! Handler::pth_is_initialized) {
        printf("sem_init is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("sem_init initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->sem_init(ret_addr, param0, param1, param2);
}

int pthread_mutex_trylock(pthread_mutex_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutex_trylock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutex_trylock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutex_trylock(ret_addr, param0);
}

int pthread_rwlockattr_init(pthread_rwlockattr_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_rwlockattr_init is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_rwlockattr_init initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_rwlockattr_init(ret_addr, param0);
}

int pthread_mutexattr_setpshared(pthread_mutexattr_t * param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutexattr_setpshared is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutexattr_setpshared initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutexattr_setpshared(ret_addr, param0, param1);
}

int pthread_rwlockattr_destroy(pthread_rwlockattr_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_rwlockattr_destroy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_rwlockattr_destroy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_rwlockattr_destroy(ret_addr, param0);
}

int pthread_barrier_destroy(pthread_barrier_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_barrier_destroy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_barrier_destroy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_barrier_destroy(ret_addr, param0);
}

int pthread_getschedparam(pthread_t param0, int * param1, struct sched_param * param2) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_getschedparam is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_getschedparam initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_getschedparam(ret_addr, param0, param1, param2);
}

int pthread_attr_getschedparam(const pthread_attr_t * param0, struct sched_param * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_getschedparam is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_getschedparam initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_getschedparam(ret_addr, param0, param1);
}

int pthread_cancel(pthread_t param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_cancel is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_cancel initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_cancel(ret_addr, param0);
}

int pthread_cond_wait(pthread_cond_t * param0, pthread_mutex_t * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_cond_wait is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_cond_wait initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_cond_wait(ret_addr, param0, param1);
}

int pthread_condattr_getpshared(const pthread_condattr_t * param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_condattr_getpshared is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_condattr_getpshared initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_condattr_getpshared(ret_addr, param0, param1);
}

int pthread_spin_trylock(pthread_spinlock_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_spin_trylock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_spin_trylock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_spin_trylock(ret_addr, param0);
}

int pthread_attr_getinheritsched(const pthread_attr_t * param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_getinheritsched is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_getinheritsched initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_getinheritsched(ret_addr, param0, param1);
}

int pthread_attr_setstack(pthread_attr_t * param0, void * param1, size_t param2) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_setstack is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_setstack initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_setstack(ret_addr, param0, param1, param2);
}

int pthread_cond_signal(pthread_cond_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_cond_signal is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_cond_signal initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_cond_signal(ret_addr, param0);
}

int sem_getvalue(sem_t * param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("sem_getvalue is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("sem_getvalue initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->sem_getvalue(ret_addr, param0, param1);
}

int pthread_mutexattr_init(pthread_mutexattr_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutexattr_init is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutexattr_init initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutexattr_init(ret_addr, param0);
}

int pthread_barrier_wait(pthread_barrier_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_barrier_wait is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_barrier_wait initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_barrier_wait(ret_addr, param0);
}

int pthread_spin_destroy(pthread_spinlock_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_spin_destroy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_spin_destroy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_spin_destroy(ret_addr, param0);
}

int pthread_key_delete(pthread_key_t param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_key_delete is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_key_delete initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_key_delete(ret_addr, param0);
}

int pthread_setspecific(pthread_key_t param0, const void * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_setspecific is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_setspecific initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_setspecific(ret_addr, param0, param1);
}

int pthread_attr_setstacksize(pthread_attr_t * param0, size_t param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_setstacksize is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_setstacksize initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_setstacksize(ret_addr, param0, param1);
}

int pthread_setcancelstate(int param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_setcancelstate is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_setcancelstate initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_setcancelstate(ret_addr, param0, param1);
}

int pthread_barrierattr_setpshared(pthread_barrierattr_t * param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_barrierattr_setpshared is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_barrierattr_setpshared initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_barrierattr_setpshared(ret_addr, param0, param1);
}

int pthread_rwlock_timedrdlock(pthread_rwlock_t * param0, const struct timespec * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_rwlock_timedrdlock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_rwlock_timedrdlock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_rwlock_timedrdlock(ret_addr, param0, param1);
}

int pthread_detach(pthread_t param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_detach is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_detach initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_detach(ret_addr, param0);
}

int usleep(useconds_t param0) {
    if (! Handler::pth_is_initialized) {
        printf("usleep is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("usleep initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->usleep(ret_addr, param0);
}

int pthread_rwlock_destroy(pthread_rwlock_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_rwlock_destroy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_rwlock_destroy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_rwlock_destroy(ret_addr, param0);
}

void pthread_exit(void * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_exit is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_exit initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    pHandler->thrille_exit(ret_addr, param0);
    printf("ThrilleErr: problem with pthread_exit\n");
    _Exit(UNRECOVERABLE_ERROR);
}

int pthread_attr_setdetachstate(pthread_attr_t * param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_setdetachstate is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_setdetachstate initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_setdetachstate(ret_addr, param0, param1);
}

int pthread_rwlock_trywrlock(pthread_rwlock_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_rwlock_trywrlock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_rwlock_trywrlock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_rwlock_trywrlock(ret_addr, param0);
}

int pthread_attr_init(pthread_attr_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_init is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_init initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_init(ret_addr, param0);
}

int pthread_attr_setguardsize(pthread_attr_t * param0, size_t param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_setguardsize is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_setguardsize initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_setguardsize(ret_addr, param0, param1);
}

int pthread_condattr_setpshared(pthread_condattr_t * param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_condattr_setpshared is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_condattr_setpshared initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_condattr_setpshared(ret_addr, param0, param1);
}

int pthread_mutex_setprioceiling(pthread_mutex_t * param0, int param1, int * param2) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutex_setprioceiling is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutex_setprioceiling initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutex_setprioceiling(ret_addr, param0, param1, param2);
}

int pthread_mutexattr_getprotocol(const pthread_mutexattr_t * param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutexattr_getprotocol is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutexattr_getprotocol initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutexattr_getprotocol(ret_addr, param0, param1);
}

int pthread_attr_getstack(const pthread_attr_t * param0, void ** param1, size_t * param2) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_getstack is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_getstack initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_getstack(ret_addr, param0, param1, param2);
}

int pthread_condattr_init(pthread_condattr_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_condattr_init is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_condattr_init initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_condattr_init(ret_addr, param0);
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t * param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutexattr_gettype is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutexattr_gettype initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutexattr_gettype(ret_addr, param0, param1);
}

int sem_post(sem_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("sem_post is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("sem_post initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->sem_post(ret_addr, param0);
}

int pthread_attr_getstacksize(const pthread_attr_t * param0, size_t * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_getstacksize is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_getstacksize initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_getstacksize(ret_addr, param0, param1);
}

int sem_timedwait(sem_t * param0, const struct timespec * param1) {
    if (! Handler::pth_is_initialized) {
        printf("sem_timedwait is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("sem_timedwait initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->sem_timedwait(ret_addr, param0, param1);
}

int pthread_equal(pthread_t param0, pthread_t param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_equal is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_equal initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_equal(ret_addr, param0, param1);
}

int pthread_spin_init(pthread_spinlock_t * param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_spin_init is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_spin_init initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_spin_init(ret_addr, param0, param1);
}

int sched_yield() throw () {
    if (! Handler::pth_is_initialized) {
        printf("sched_yield is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("sched_yield initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->sched_yield(ret_addr);
}

int pthread_key_create(pthread_key_t * param0, void (* param1)(void *)) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_key_create is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_key_create initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_key_create(ret_addr, param0, param1);
}

int pthread_mutexattr_destroy(pthread_mutexattr_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutexattr_destroy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutexattr_destroy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutexattr_destroy(ret_addr, param0);
}

int pthread_cond_timedwait(pthread_cond_t * param0, pthread_mutex_t * param1, const struct timespec * param2) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_cond_timedwait is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_cond_timedwait initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_cond_timedwait(ret_addr, param0, param1, param2);
}

int pthread_condattr_getclock(const pthread_condattr_t * param0, clockid_t * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_condattr_getclock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_condattr_getclock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_condattr_getclock(ret_addr, param0, param1);
}

int pthread_setschedparam(pthread_t param0, int param1, const struct sched_param * param2) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_setschedparam is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_setschedparam initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_setschedparam(ret_addr, param0, param1, param2);
}

int pthread_attr_setinheritsched(pthread_attr_t * param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_setinheritsched is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_setinheritsched initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_setinheritsched(ret_addr, param0, param1);
}

int pthread_mutex_unlock(pthread_mutex_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutex_unlock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutex_unlock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutex_unlock(ret_addr, param0);
}

int pthread_attr_setschedparam(pthread_attr_t * param0, const struct sched_param * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_setschedparam is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_setschedparam initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_setschedparam(ret_addr, param0, param1);
}

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t * param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_rwlockattr_setpshared is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_rwlockattr_setpshared initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_rwlockattr_setpshared(ret_addr, param0, param1);
}

int pthread_attr_setstackaddr(pthread_attr_t * param0, void * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_setstackaddr is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_setstackaddr initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_setstackaddr(ret_addr, param0, param1);
}

int pthread_mutex_destroy(pthread_mutex_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutex_destroy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutex_destroy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutex_destroy(ret_addr, param0);
}

int pthread_attr_getschedpolicy(const pthread_attr_t * param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_getschedpolicy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_getschedpolicy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_getschedpolicy(ret_addr, param0, param1);
}

int pthread_setcanceltype(int param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_setcanceltype is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_setcanceltype initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_setcanceltype(ret_addr, param0, param1);
}

int sigwait(const sigset_t * param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("sigwait is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("sigwait initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->sigwait(ret_addr, param0, param1);
}

int pthread_rwlock_init(pthread_rwlock_t * param0, const pthread_rwlockattr_t * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_rwlock_init is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_rwlock_init initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_rwlock_init(ret_addr, param0, param1);
}

int pthread_rwlock_unlock(pthread_rwlock_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_rwlock_unlock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_rwlock_unlock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_rwlock_unlock(ret_addr, param0);
}

int pthread_mutexattr_setprotocol(pthread_mutexattr_t * param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutexattr_setprotocol is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutexattr_setprotocol initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutexattr_setprotocol(ret_addr, param0, param1);
}

int pthread_mutex_getprioceiling(const pthread_mutex_t * param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutex_getprioceiling is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutex_getprioceiling initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutex_getprioceiling(ret_addr, param0, param1);
}

int sem_trywait(sem_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("sem_trywait is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("sem_trywait initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->sem_trywait(ret_addr, param0);
}

int pthread_attr_getstackaddr(const pthread_attr_t * param0, void ** param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_getstackaddr is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_getstackaddr initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_getstackaddr(ret_addr, param0, param1);
}

int pthread_mutexattr_settype(pthread_mutexattr_t * param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutexattr_settype is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutexattr_settype initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutexattr_settype(ret_addr, param0, param1);
}

int pthread_getcpuclockid(pthread_t param0, clockid_t * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_getcpuclockid is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_getcpuclockid initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_getcpuclockid(ret_addr, param0, param1);
}

int pthread_cond_destroy(pthread_cond_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_cond_destroy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_cond_destroy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_cond_destroy(ret_addr, param0);
}

int pthread_spin_lock(pthread_spinlock_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_spin_lock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_spin_lock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_spin_lock(ret_addr, param0);
}

int pthread_once(pthread_once_t * param0, void (* param1)(void)) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_once is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_once initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_once(ret_addr, param0, param1);
}

int pthread_atfork(void (* param0)(void), void (* param1)(void), void(* param2)(void)) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_atfork is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_atfork initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_atfork(ret_addr, param0, param1, param2);
}

int pthread_attr_setscope(pthread_attr_t * param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_setscope is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_setscope initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_setscope(ret_addr, param0, param1);
}

void * pthread_getspecific(pthread_key_t param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_getspecific is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_getspecific initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_getspecific(ret_addr, param0);
}

int pthread_attr_destroy(pthread_attr_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_destroy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_destroy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_destroy(ret_addr, param0);
}

int pthread_mutex_timedlock(pthread_mutex_t * param0, const struct timespec * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutex_timedlock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutex_timedlock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutex_timedlock(ret_addr, param0, param1);
}

int pthread_barrier_init(pthread_barrier_t * param0, const pthread_barrierattr_t * param1, unsigned param2) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_barrier_init is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_barrier_init initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_barrier_init(ret_addr, param0, param1, param2);
}

int pthread_setschedprio(pthread_t param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_setschedprio is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_setschedprio initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_setschedprio(ret_addr, param0, param1);
}

int pthread_condattr_setclock(pthread_condattr_t * param0, clockid_t param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_condattr_setclock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_condattr_setclock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_condattr_setclock(ret_addr, param0, param1);
}

int pthread_cond_broadcast(pthread_cond_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_cond_broadcast is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_cond_broadcast initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_cond_broadcast(ret_addr, param0);
}

int pthread_spin_unlock(pthread_spinlock_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_spin_unlock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_spin_unlock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_spin_unlock(ret_addr, param0);
}

int pthread_rwlock_rdlock(pthread_rwlock_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_rwlock_rdlock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_rwlock_rdlock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_rwlock_rdlock(ret_addr, param0);
}

int pthread_rwlock_wrlock(pthread_rwlock_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_rwlock_wrlock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_rwlock_wrlock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_rwlock_wrlock(ret_addr, param0);
}

int pthread_join(pthread_t param0, void ** param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_join is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_join initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_join(ret_addr, param0, param1);
}

int pthread_cond_init(pthread_cond_t * param0, const pthread_condattr_t * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_cond_init is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_cond_init initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_cond_init(ret_addr, param0, param1);
}

int pthread_attr_getguardsize(const pthread_attr_t * param0, size_t * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_getguardsize is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_getguardsize initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_getguardsize(ret_addr, param0, param1);
}

int pthread_barrierattr_init(pthread_barrierattr_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_barrierattr_init is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_barrierattr_init initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_barrierattr_init(ret_addr, param0);
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_rwlock_tryrdlock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_rwlock_tryrdlock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_rwlock_tryrdlock(ret_addr, param0);
}

int pthread_attr_setschedpolicy(pthread_attr_t * param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_setschedpolicy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_setschedpolicy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_setschedpolicy(ret_addr, param0, param1);
}

int pthread_barrierattr_destroy(pthread_barrierattr_t * param0) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_barrierattr_destroy is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_barrierattr_destroy initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_barrierattr_destroy(ret_addr, param0);
}

pthread_t pthread_self() __THROW {
    if (! Handler::pth_is_initialized) {
        printf("pthread_self is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_self initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_self(ret_addr);
}

int pthread_attr_getdetachstate(const pthread_attr_t * param0, int * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_attr_getdetachstate is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_attr_getdetachstate initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_attr_getdetachstate(ret_addr, param0, param1);
}

int pthread_rwlock_timedwrlock(pthread_rwlock_t * param0, const struct timespec * param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_rwlock_timedwrlock is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_rwlock_timedwrlock initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_rwlock_timedwrlock(ret_addr, param0, param1);
}

int pthread_mutexattr_setprioceiling(pthread_mutexattr_t * param0, int param1) {
    if (! Handler::pth_is_initialized) {
        printf("pthread_mutexattr_setprioceiling is calling pth_start\n");
        pth_start();
        if (! Handler::pth_is_initialized) {
            printf("pthread_mutexattr_setprioceiling initialization fail\n");
            _Exit(UNRECOVERABLE_ERROR);
        }
    }

    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_mutexattr_setprioceiling(ret_addr, param0, param1);
}



