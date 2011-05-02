/**
 * TODO
 * -put in locks to synch memory
 *  
 */

#define UNLOCKASSERT
#include "executiontracker.h"

void ExecutionTracker::constructorHelper() {
    log = new Logger("my-schedule", "thrille-sched");
    bt = new BarrierTracker();
    global_lock = new pthread_mutex_t;
    signal_cond = new pthread_cond_t;
    Originals::pthread_mutex_init(global_lock, NULL);
    Originals::pthread_cond_init(signal_cond, NULL);

    lock_count = 0;
    preemptions = 0;
    nonpreemptions = 0;
    contextswitch = 0;
    skipped_comparisons = 0;
}

ExecutionTracker::ExecutionTracker(thrID myself) {
    constructorHelper();
    main_thread_id = myself;
    initializeThreadData(myself);
}

void ExecutionTracker::segfault(thrID myself) {
    log->segfault(myself);
    destructorHelper();
}

ExecutionTracker::ExecutionTracker() {
    constructorHelper();
}


bool ExecutionTracker::BeforeGetschedparam(thrID myself, 
        void * ret_addr, 
        pthread_t arg0, 
        int * arg1, 
        struct sched_param * arg2) {
    return true;
}

bool ExecutionTracker::BeforeKeyCreate(thrID myself, void * ret_addr,
        pthread_key_t * key, void (*rt)(void*)) {
    return true;
}


bool ExecutionTracker::BeforeSetspecific(thrID myself, void * ret_addr,
        pthread_key_t arg0, const void * arg1) {
    return true;
}


bool ExecutionTracker::BeforeGetspecific(thrID myself, void * ret_addr,
        pthread_key_t) {
    return true;
}

bool ExecutionTracker::BeforeAttrSetstacksize(thrID myself, void * ret_addr,
        pthread_attr_t * arg0, size_t arg1) {
    return true;
}

bool ExecutionTracker::BeforeAttrSetdetachstate(thrID myself, void * ret_addr,
        pthread_attr_t * arg0, int arg1) {
    return true;
}

bool ExecutionTracker::BeforeAttrGetschedparam(thrID myself, void * ret_addr, 
        const pthread_attr_t * arg0, struct sched_param * arg1) {
    return true;
}

bool ExecutionTracker::BeforeAttrSetschedparam(thrID myself, void * ret_addr,
        pthread_attr_t * attr, const struct sched_param * myp) {
    bool isNormalSchedulingPriority = false;
    if ( myp->sched_priority == SCHED_FIFO) { 
        safe_assert(isNormalSchedulingPriority);
    }
    else if ( myp->sched_priority == SCHED_RR) { 
        safe_assert(isNormalSchedulingPriority);
    }
    /* SCHED_SPORADIC not declared in potus' libraries
     *
     * else if ( myp->sched_priority == SCHED_SPORADIC) { 
     *    safe_assert(isNormalSchedulingPriority);
     * }
     */
    return true;
}

bool ExecutionTracker::BeforeEqual(thrID myself, void * ret_addr,
        pthread_t arg0, pthread_t arg1) {
    return true;
}

bool ExecutionTracker::BeforeAttrDestroy(thrID myself, void * ret_addr,
        pthread_attr_t * arg0) {
    return true;
}

bool ExecutionTracker::BeforeOnce(thrID myself, void * ret_addr, 
        pthread_once_t * once, void (* param1) (void)) {
    return true;
}


bool ExecutionTracker::BeforeMutexTrylock(thrID myself, void * ret_addr,
        pthread_mutex_t * lock) {
    glock();
    log->BeforeMutexTrylock(myself);
    enableThread(myself);
    handleBeforeMutexLock(myself, ret_addr, lock, IS_NOT_COND_WAIT);
    SchedPointInfo tmp(ret_addr, "Before Mutex Try Lock", myself, 
            enable_map, IS_NOT_YIELD, lock);
    schedule(&tmp);
    gunlock();
    pauseThread(myself);
    return false;
}

int ExecutionTracker::SimulateMutexTrylock(thrID myself, void * ret_addr,
        pthread_mutex_t * lock) {
    glock();
    log->SimulateMutexTrylock(myself);
    int result = Originals::pthread_mutex_trylock(lock);
    if (result == OPERATION_SUCCESSFUL) {
        threadHasAcquiredLock(myself, lock);
    }
    gunlock();
    return result;
}

void ExecutionTracker::AfterMutexTrylock(thrID myself, void * ret_addr,
        int ret_val, pthread_mutex_t * lock) {
    return;
}

void ExecutionTracker::checkForDanglingThreads() {
    map<thrID, bool>::iterator itr;
    for (itr = finish_map.begin(); itr != finish_map.end(); itr++) {
        thrID thread_handle = itr->first;
        if (thread_handle == main_thread_id) {
            bool mainThreadIsNotFinished = ! itr->second;
            safe_assert(mainThreadIsNotFinished);
        } else {
            bool threadIsFinished = itr->second;
            if (! threadIsFinished) {
                printf("Thrille: WARNING thread %d is not finished\n",
                        thread_handle);
            }
        }
    }
}

void ExecutionTracker::destructorHelper() {
    printf("Number of Context Switches: %d\n", contextswitch);
    printf("Number of Preemptions: %d\n", preemptions);
    printf("Number of Non-Preemptive Context Switches: %d\n", nonpreemptions);
    if (skipped_comparisons > 0) {
        printf("WARNING: skipped %d schedule ", skipped_comparisons);
        printf("id comparisons because it appears the scheduling point ");
        printf("occurred in a dynamically loaded library\n");
    }
}




void ExecutionTracker::thrilleCheckpoint(thrID myself, void * ret_addr) {
    glock();
    printf("Thrille: CHECKPOINT CALLED!\n");
    SchedPointInfo tmp(ret_addr, "Thrille Checkpoint", myself, 
            enable_map, IS_NOT_YIELD);
    schedule(&tmp);
    gunlock();
    pauseThread(myself);
    return;
}

void ExecutionTracker::thrilleAssert(thrID myself,
        void * ret_addr, bool cond) {
    if (!cond) {
        log->programAssertFail(myself, ret_addr);
        destructorHelper();
        _Exit(UNRECOVERABLE_ERROR);
    }
}


void ExecutionTracker::thrilleSchedPoint(thrID myself, void * ret_addr) {
    glock();
    log->BeforeSchedPoint(myself);
    safe_assert(isEnabled(myself));
    SchedPointInfo tmp(ret_addr, "Before Generic Schedule Point", myself,
            enable_map, IS_NOT_YIELD);
    schedule(&tmp);
    gunlock();
    pauseThread(myself);
}

ExecutionTracker::~ExecutionTracker() {
    checkForDanglingThreads();
    destructorHelper();
    delete log;
    delete global_lock;
    delete bt;
    delete signal_cond;
}


bool ExecutionTracker::BeforeMutexattrSettype(thrID myself,
        void * ret_addr,
        pthread_mutexattr_t * attr,
        int type) {
    if (type == PTHREAD_MUTEX_NORMAL)  {
        return true;
    }
    bool goodMutexattr = false;
    safe_assert(goodMutexattr);
    return false;
}


bool ExecutionTracker::BeforeBarrierDestroy(thrID myself,
        void * ret_addr, pthread_barrier_t * barrier) {
    return false;
}


int ExecutionTracker::SimulateBarrierDestroy(thrID myself,
        void * ret_addr, pthread_barrier_t * barrier) {
    glock();
    barrier_destroy.push_back(barrier);
    int ret_val = Originals::pthread_barrier_destroy(barrier);
    gunlock();
    return ret_val;
}


void ExecutionTracker::checkBarrierValidity(pthread_barrier_t * barrier) {
    vector<pthread_barrier_t *>::iterator itr;
    for (itr = barrier_destroy.begin(); itr != barrier_destroy.end(); itr++) {
        if (barrier == (*itr)) {
            bool barrierHasNotBeenDestroyed = false;
            safe_assert(barrierHasNotBeenDestroyed);
        }
    }
}


void ExecutionTracker::barrierIsValid(pthread_barrier_t * barrier) {
    vector<pthread_barrier_t *>::iterator itr;
    bool keepGoing = true;
    while (keepGoing) {
        keepGoing = false;
        for (itr = barrier_destroy.begin();
                itr != barrier_destroy.end(); itr++) {
            if (barrier == (*itr)) {
                barrier_destroy.erase(itr);
                keepGoing = true;
                break;
            }
        }
    }
}


bool ExecutionTracker::BeforeMutexInit(thrID myself, void * ret_addr,
        pthread_mutex_t * lock,
        const pthread_mutexattr_t * lockattr) {
    lockIsValid(lock);
    return true;
}

bool ExecutionTracker::BeforeCondInit(thrID myself, void * ret_addr,
        pthread_cond_t * cond,
        const pthread_condattr_t * condattr) {
    if (condattr != NULL) {
        bool condAttrIsNull = false;
        safe_assert(condAttrIsNull);
    }
    condIsValid(cond);
    return true;
}

bool ExecutionTracker::BeforeSelf(thrID myself, void * ret_addr) {
    return true;
}


bool ExecutionTracker::BeforeSemDestroy(thrID myself, void * ret_addr,
        sem_t * sem) {
    return false;
}

int ExecutionTracker::SimulateSemDestroy(thrID myself, void * ret_addr, 
        sem_t * sem) {
    glock();
    sem_destroy.push_back(sem);
    int ret_val = Originals::sem_destroy(sem);
    gunlock();
    return ret_val;
}

void ExecutionTracker::checkSemValidity(sem_t * sem) {
    vector<sem_t *>::iterator itr;
    for (itr = sem_destroy.begin(); itr != sem_destroy.end(); itr++) {
        if (sem == (*itr)) {
            bool semHasNotBeenDestroyed = false;
            safe_assert(semHasNotBeenDestroyed);
        }
    }
}

void ExecutionTracker::semIsValid(sem_t * sem) {
    vector<sem_t *>::iterator itr;
    bool keepGoing = true;
    while (keepGoing) {
        keepGoing = false;
        for (itr = sem_destroy.begin(); itr != sem_destroy.end(); itr++) {
            if (sem == (*itr)) {
                sem_destroy.erase(itr);
                keepGoing = true;
                break;
            }
        }
    }
}

void ExecutionTracker::ThreadStart(thrID myself,
        void * (*start_routine) (void *), void * arg) {
    glock();
    log->ThreadStart(myself, arg);
    handleThreadStart(myself, start_routine);
    gunlock();
    pauseThread(myself);
}

void ExecutionTracker::ThreadFinish(thrID myself,
        void * (*start_routine) (void *), void * arg) {
    glock();
    log->ThreadFinish(myself, arg);
    finalizeThread(myself, reinterpret_cast<void *>(start_routine)); 
    lock_count--;
    Originals::pthread_mutex_unlock(global_lock);
}

bool ExecutionTracker::BeforeCondDestroy(thrID myself,
        void * ret_addr, pthread_cond_t * cond) {
    return false;
}

int ExecutionTracker::SimulateCondDestroy(thrID myself,
        void * ret_addr, pthread_cond_t * cond) {
    glock();
    cond_destroy.push_back(cond);
    int ret_val = Originals::pthread_cond_destroy(cond);
    gunlock();
    return ret_val;
}

bool ExecutionTracker::BeforeMutexDestroy(thrID myself, void * ret_addr,
        pthread_mutex_t * mutex) {
    return false;
}

int ExecutionTracker::SimulateMutexDestroy(thrID myself, void * addr,
        pthread_mutex_t * lock) {
    glock();
    mutex_destroy.push_back(lock);
    int ret_val = Originals::pthread_mutex_destroy(lock);
    gunlock();
    return ret_val;
}

bool ExecutionTracker::BeforeAttrInit(thrID, void *, pthread_attr_t *) {
    return true;
}


bool ExecutionTracker::BeforeAttrSetscope(thrID, void *,
        pthread_attr_t *, int) {
    return true;
}

bool ExecutionTracker::BeforeCreate(thrID myself,
        thrID child,
        void * ret_addr,
        pthread_t * thread, 
        const pthread_attr_t * thread_attr,
        void *(* routine)(void *),
        void * arg) {
    glock();
    log->BeforeCreate(myself, child);
    initializeThreadData(child);
    gunlock();
    return true;
}

void ExecutionTracker::AfterCreate(thrID myself,
        thrID child,
        void * ret_addr,
        int ret_val,
        pthread_t * thread,
        const pthread_attr_t * thread_attr,
        void *(* routine)(void *),
        void * arg) {
    glock();
    log->AfterCreate(myself);
    handleAfterCreate(myself, ret_addr);
    SchedPointInfo tmp(ret_addr, "After Create", myself,
            enable_map, IS_NOT_YIELD);
    schedule(&tmp);
    gunlock();
    pauseThread(myself);
}

bool ExecutionTracker::BeforeJoin(thrID myself,
        thrID target,
        void * ret_addr,
        pthread_t thread, 
        void** arg) {
    glock();
    log->BeforeJoin(myself, target);
    safe_assert(target != INVALID_THREAD);
    if (threadIsFinished(target)) {
        enableThread(myself);
    } else {
        disableThread(myself);
        registerThreadJoin(target, myself);
    }
    handleBeforeJoin(myself, target, ret_addr);
    SchedPointInfo tmp(ret_addr, "Before Join", myself,
            enable_map, IS_NOT_YIELD, (void*) target);
    schedule(&tmp);
    gunlock();
    pauseThread(myself);
    glock();
    gunlock();
    return true;
}

void ExecutionTracker::addThreadWaitingOnLock(thrID target, 
        pthread_mutex_t * lock) {
    lock_wait_map[target] = lock;
}

void ExecutionTracker::addThreadReadyOnLock(thrID target,
        pthread_mutex_t * lock) {
    lock_ready_map[target] = lock;
}

void ExecutionTracker::threadHasAcquiredLock(thrID target, 
        pthread_mutex_t * lock) {
    lock_ready_map[target] = NULL;
    map<thrID, pthread_mutex_t *>::iterator itr;
    for (itr = lock_ready_map.begin(); itr != lock_ready_map.end(); itr++) {
        if (itr->second == lock) {
            disableThread(itr->first);
            lock_wait_map[itr->first] = lock;
            lock_ready_map[itr->first] = NULL;
        }
    }
}

void ExecutionTracker::determineLockStatus(thrID myself, 
        pthread_mutex_t * lock) {

    int result = Originals::pthread_mutex_trylock(lock);

    if (result == EBUSY) {
        disableThread(myself);
        addThreadWaitingOnLock(myself, lock);
    } else if (result == OPERATION_SUCCESSFUL) {
        Originals::pthread_mutex_unlock(lock);
        enableThread(myself);
        addThreadReadyOnLock(myself, lock);
    } else {
        /* potentially invalid mutex */
        Originals::pthread_mutex_unlock(lock);
        enableThread(myself);
        addThreadReadyOnLock(myself, lock);
    }

}

bool ExecutionTracker::BeforeMutexLock(thrID myself, void * ret_addr,
        pthread_mutex_t * lock, bool isWait) {
    if (! isWait) {
        glock();
    }

    /* Lock and unlocks get called when a thread is finishing up */
    if (threadIsFinished(myself)) {
        if (! isWait) {
            gunlock();
        }
        return true;
    }
    log->BeforeMutexLock(myself, ret_addr, lock, isWait);
    determineLockStatus(myself, lock);
    handleBeforeMutexLock(myself, ret_addr, lock, isWait);
    if (! isWait) {
        SchedPointInfo tmp(ret_addr, "Before Mutex Lock", myself, 
                enable_map, IS_NOT_YIELD, lock);
        schedule(&tmp);
        gunlock();
        pauseThread(myself);
    }
    return false;
}

        
void ExecutionTracker::checkCondValidity(pthread_cond_t * cond) {
    vector<pthread_cond_t *>::iterator itr;
    for (itr = cond_destroy.begin(); itr != cond_destroy.end(); itr++) {
        if (cond == (*itr)) {
            bool condHasNotBeenDestroyed = false;
            safe_assert(condHasNotBeenDestroyed);
        }
    }
}


void ExecutionTracker::condIsValid(pthread_cond_t * cond) {
    vector<pthread_cond_t *>::iterator itr;
    bool keepGoing = true;
    while (keepGoing) {
        keepGoing = false;
        for (itr = cond_destroy.begin(); itr != cond_destroy.end(); itr++) {
            if (cond == (*itr)) {
                cond_destroy.erase(itr);
                keepGoing = true;
                break;
            }
        }
    }
}

void ExecutionTracker::lockIsValid(pthread_mutex_t * lock) {
    vector<pthread_mutex_t *>::iterator itr;
    bool keepGoing = true;
    while (keepGoing) {
        keepGoing = false;
        for (itr = mutex_destroy.begin(); itr != mutex_destroy.end(); itr++) {
            if (lock == (*itr)) {
                mutex_destroy.erase(itr);
                keepGoing = true;
                break;
            }
        }
    }
}

int ExecutionTracker::SimulateMutexLock(thrID myself, void * ret_addr,
        pthread_mutex_t * lock, bool isWait) {
    if (! isWait) {
        glock();
    }
    log->SimulateMutexLock(myself, isWait);
    threadHasAcquiredLock(myself, lock);
    int result = Originals::pthread_mutex_trylock(lock);
    if (! isWait) {
        gunlock();
    }
    return result;
}

void ExecutionTracker::threadHasReleasedLock(thrID myself, 
        pthread_mutex_t * lock) {
    map<thrID, pthread_mutex_t *>::iterator itr;
    for (itr = lock_wait_map.begin(); itr != lock_wait_map.end(); itr++) {
        if (itr->second == lock) {
            enableThread(itr->first);
            lock_ready_map[itr->first] = lock;
            lock_wait_map[itr->first] = NULL;
        }
    }
}

bool ExecutionTracker::BeforeMutexUnlock(thrID myself, void * ret_addr,
        pthread_mutex_t * lock, 
        bool isWait) {
    return true;
}

void ExecutionTracker::AfterMutexUnlock(thrID myself, 
        void * ret_addr,
        int ret_val, 
        pthread_mutex_t * lock,
        bool isWait) {
    if (!isWait) {
        glock(); 
    }

    /* Lock and unlocks get called when a thread is finishing up */
    if (threadIsFinished(myself)) {
        if (!isWait) {
            gunlock();
        }
        return;
    }
    log->AfterMutexUnlock(myself, isWait);
    threadHasReleasedLock(myself, lock);
    handleAfterMutexUnlock(myself, ret_addr, lock, isWait);
    if (! isWait) {
        SchedPointInfo tmp(ret_addr, "After Mutex Unlock", myself,
                enable_map, IS_NOT_YIELD, lock);
        schedule(&tmp);
        gunlock();
        pauseThread(myself); 
    }
}

bool ExecutionTracker::BeforeCondWait(thrID myself,
        void * ret_addr,
        pthread_cond_t * cond, 
        pthread_mutex_t * lock) {
    glock();
    log->BeforeCondWait(myself);
    disableThread(myself);
    gunlock();
    return false;
}

void ExecutionTracker::waitPauseThread(thrID target) {
    glock();
    sem_t * pause = wait_sem_map[target];
    safe_assert(pause != NULL);
    log->waitPauseThread(target);
    gunlock();
    int result;
    do {
        result = Originals::sem_wait(pause);
    } while (result == -1 && errno == EINTR);
    safe_assert(result == 0);
}

//TODO: locks or no?
void ExecutionTracker::waitWakeThread(thrID target) {
    log->waitWakeThread(target);
    sem_t * pause = wait_sem_map[target];
    safe_assert(pause != NULL);
    safe_assert(Originals::sem_post(pause) == OPERATION_SUCCESSFUL);
}

void ExecutionTracker::signalThreadReady(thrID myself) {
    Originals::pthread_mutex_lock(global_lock);
    lock_count++;
    wait_ready_map[myself] = true;
    Originals::pthread_cond_signal(signal_cond);
    lock_count--;
    Originals::pthread_mutex_unlock(global_lock);
}

int ExecutionTracker::SimulateCondWait(thrID myself,
        void * ret_addr,
        pthread_cond_t * cond, 
        pthread_mutex_t * lock) {
    glock();
    log->SimulateCondWait(myself);
    checkCondValidity(cond);
    //unlock
    bool proceed = BeforeMutexUnlock(myself, ret_addr, lock, IS_COND_WAIT);
    safe_assert(proceed);
    safe_assert(Originals::pthread_mutex_unlock(lock) == OPERATION_SUCCESSFUL);
    AfterMutexUnlock(myself, ret_addr, OPERATION_SUCCESSFUL,lock,IS_COND_WAIT);

    cond_map[myself] = cond;
    handleSimulateCondWait(myself, ret_addr, cond, lock);
    SchedPointInfo tmp(ret_addr, "Simulate Cond Wait", myself,
            enable_map, IS_NOT_YIELD, cond, lock);
    schedule(&tmp);
    gunlock();

    waitPauseThread(myself);

    glock();
    cond_map[myself] = NULL;
    gunlock();

    //lock
    proceed = BeforeMutexLock(myself, ret_addr, lock, IS_COND_WAIT);
    safe_assert(proceed == false);
    signalThreadReady(myself);
    pauseThread(myself);
    SimulateMutexLock(myself, ret_addr, lock, IS_COND_WAIT);
    glock();
    gunlock();
    return OPERATION_SUCCESSFUL;
}


bool ExecutionTracker::BeforeCondTimedwait(thrID myself, 
                void * ret_addr,
                pthread_cond_t * cond, 
                pthread_mutex_t * lock,
                const struct timespec * time) {
    glock();
    log->BeforeCondTimedwait(myself);
    enableThread(myself);
    gunlock();
    return false;
}

int ExecutionTracker::SimulateCondTimedwait(thrID myself, 
                void * ret_addr,
                pthread_cond_t * cond, 
                pthread_mutex_t * lock,
                const struct timespec * time) {
    glock();
    log->SimulateCondTimedwait(myself);
    checkCondValidity(cond);
    
    //unlock
    bool proceed = BeforeMutexUnlock(myself, ret_addr, lock, IS_COND_WAIT);
    safe_assert(proceed);
    safe_assert(Originals::pthread_mutex_unlock(lock) == OPERATION_SUCCESSFUL);
    AfterMutexUnlock(myself, ret_addr, OPERATION_SUCCESSFUL,lock,IS_COND_WAIT);
    
    
    cond_map[myself] = cond;
    handleSimulateCondTimedwait(myself, ret_addr, cond, lock);
    SchedPointInfo tmp(ret_addr, "Simulate Timed Wait (s)",
            myself, enable_map, IS_YIELD, cond, lock);
    schedule(&tmp);
    gunlock();

    waitPauseThread(myself);

    glock();
    cond_map[myself] = NULL;
    gunlock();

    //lock
    proceed = BeforeMutexLock(myself, ret_addr, lock, IS_COND_WAIT);
    safe_assert(proceed == false);
    signalThreadReady(myself);
    glock();
    bool was_signalled = was_signalled_map[myself];
    if (! was_signalled) {
        SchedPointInfo tmp(ret_addr, "Simulate Timed Wait (e)",
                myself, enable_map, IS_NOT_YIELD, cond, lock);
        schedule(&tmp);
        was_signalled_map[myself] = true;
    }
    gunlock();
    pauseThread(myself);
    SimulateMutexLock(myself, ret_addr, lock, IS_COND_WAIT);
    if (was_signalled) {
        log->timedwaitNoTimeout(myself);
        glock();
        gunlock();
        return OPERATION_SUCCESSFUL;
    } else {
        log->timedwaitTimeout(myself);
        glock();
        gunlock();
        return ETIMEDOUT;        
    }
}


bool ExecutionTracker::BeforeCondSignal(thrID myself, void * ret_addr,
        pthread_cond_t * cond) {
    glock();
    checkCondValidity(cond);
    gunlock();
    return false;
}

void ExecutionTracker::waitThreadReady(thrID myself, thrID target) {
    Originals::pthread_mutex_lock(global_lock);
    lock_count++;
    while (! wait_ready_map[target]) {
        lock_count--;
        Originals::pthread_cond_wait(signal_cond, global_lock);
        lock_count++;
    }
    lock_count--;
    Originals::pthread_mutex_unlock(global_lock);
    return;
}


void ExecutionTracker::scheduleThreadWaitingOnCond(thrID target,
        SignalPointInfo * s) {
    glock();
    safe_assert(s->cond_map[target] == s->cond);
    log->signalScheduling(target, s);
    wait_ready_map[target] = false;
    waitWakeThread(target);
    gunlock();
    waitThreadReady(s->thread, target);
}

bool ExecutionTracker::isSignalMissed(SignalPointInfo * s) {
    bool missed = true;
    map<thrID, pthread_cond_t *>::iterator itr;
    for (itr = s->cond_map.begin(); itr != s->cond_map.end(); itr++) {
        if (itr->second == s->cond) {
            missed = false;
        }
    }
    return missed;
}

thrID ExecutionTracker::selectThreadWaitingOnCond(SignalPointInfo * s) {
    map<thrID, pthread_cond_t *>::iterator itr;
    for (itr = s->cond_map.begin(); itr != s->cond_map.end(); itr++) {
        if (itr->second == s->cond) {
            return itr->first; 
        }
    }
    return INVALID_THREAD;
}

void ExecutionTracker::wakeThreadWaitingOnCond(SignalPointInfo * s) {
    bool missed = isSignalMissed(s);
    if (missed) {
        log->SignalMissed(s->thread, s->isBroadcast);
    } else {
        if (s->isBroadcast) {
            thrID newchoice = selectThreadWaitingOnCond(s);
            safe_assert(newchoice != INVALID_THREAD);
            while (newchoice != INVALID_THREAD) {
                safe_assert(s->cond_map[newchoice] == s->cond);
                scheduleThreadWaitingOnCond(newchoice, s);
                s->cond_map = cond_map;
                newchoice = selectThreadWaitingOnCond(s);
            }
        } else {
            thrID nextchoice = log->getNextSignalChoice();
            if (nextchoice == INVALID_THREAD) {
                thrID newchoice = selectThreadWaitingOnCond(s);
                safe_assert(newchoice != INVALID_THREAD);
                scheduleThreadWaitingOnCond(newchoice, s);
            } else {
                scheduleThreadWaitingOnCond(nextchoice, s);
            }
        }
    }
}

int ExecutionTracker::SimulateCondSignal(thrID myself, void * ret_addr,
        pthread_cond_t * cond) {
    SignalPointInfo tmp(ret_addr, IS_NOT_BROADCAST, myself, cond, cond_map);
    wakeThreadWaitingOnCond(&tmp);
    glock();
    enableThread(myself);
    handleSimulateCondSignal(myself, ret_addr, cond);
    SchedPointInfo tmp2(ret_addr, "Simulate Signal", myself,
            enable_map, IS_NOT_YIELD, cond);
    schedule(&tmp2);
    gunlock();
    pauseThread(myself);
    return OPERATION_SUCCESSFUL;  
}

bool ExecutionTracker::BeforeCondBroadcast(thrID myself, void * ret_addr,
        pthread_cond_t * cond) {
    glock();
    checkCondValidity(cond);
    gunlock();
    return false; 
}

int ExecutionTracker::SimulateCondBroadcast(thrID myself, void * ret_addr,
        pthread_cond_t * cond) {
    SignalPointInfo tmp(ret_addr, IS_BROADCAST, myself, cond, cond_map);
    wakeThreadWaitingOnCond(&tmp);
    glock();
    enableThread(myself);
    handleSimulateCondBroadcast(myself, ret_addr, cond);
    SchedPointInfo tmp2(ret_addr, "Simulate Broadcast", myself,
            enable_map, IS_NOT_YIELD, cond);
    schedule(&tmp2);
    gunlock();
    pauseThread(myself);
    return OPERATION_SUCCESSFUL;  
}

void ExecutionTracker::myMemoryRead(thrID myself, void * ret_addr,
        void * addr) {
    glock();
    log->myMemoryRead(myself, addr);
    handleMyMemoryRead(myself, ret_addr, addr);
    SchedPointInfo tmp(ret_addr, "Memory Read", myself,
            enable_map, IS_NOT_YIELD, addr);
    schedule(&tmp);
    gunlock();
    pauseThread(myself);
    glock();
    gunlock();
}

void ExecutionTracker::myMemoryWrite(thrID myself, void * ret_addr,
        void * addr) {
    glock();
    log->myMemoryWrite(myself, addr);
    handleMyMemoryWrite(myself, ret_addr, addr);
    SchedPointInfo tmp(ret_addr, "Memory Write", myself,
            enable_map, IS_NOT_YIELD, addr);
    schedule(&tmp);
    gunlock();
    pauseThread(myself);
    glock();
    gunlock();
}

bool ExecutionTracker::tryToSchedule() {
    return log->tryToSchedule();
}


bool ExecutionTracker::isSchedulingPoint(void * ret_addr) {
    return log->isSchedulingPoint(ret_addr);
}

bool ExecutionTracker::BeforeExit(thrID myself, void * ret_addr, void * arg) {
    glock();
    log->BeforeExit(myself);
    gunlock();
    return false;
}

void ExecutionTracker::SimulateExit(thrID myself, void * ret_addr,
        void * arg) {
    glock();
    log->SimulateExit(myself);
    finalizeThread(myself, ret_addr); 
    gunlock();
    Originals::pthread_exit(arg);
}
bool ExecutionTracker::BeforeSleep(thrID myself, void * ret_addr,
        unsigned int time) {
    return false;
}

unsigned int ExecutionTracker::SimulateSleep(thrID myself, void * ret_addr,
        unsigned int time) {
    glock();
    log->mySleep(myself);
    handleMySleep(myself, ret_addr);
    SchedPointInfo tmp(ret_addr, "Simulate Sleep",
            myself, enable_map, IS_YIELD);
    schedule(&tmp);
    gunlock();
    pauseThread(myself);
    return OPERATION_SUCCESSFUL;
}

bool ExecutionTracker::BeforeUsleep(thrID myself, void * ret_addr, 
        useconds_t time) {
    return false;
}

int ExecutionTracker::SimulateUsleep(thrID myself, void * ret_addr, 
        useconds_t time) {
    glock();
    log->myUsleep(myself);
    handleMyUsleep(myself, ret_addr);
    SchedPointInfo tmp(ret_addr, "Simulate Usleep", 
            myself, enable_map, IS_YIELD);
    schedule(&tmp);
    gunlock();
    pauseThread(myself);
    return OPERATION_SUCCESSFUL;

}

void ExecutionTracker::registerThreadJoin(thrID target, thrID joiner) {
    safe_assert(join_map[target] == UNJOINED);
    join_map[target] = joiner;
}

bool ExecutionTracker::threadIsFinished(thrID target) {
    return finish_map[target];
}


void ExecutionTracker::wakeThread(thrID target) {
    sem_t * pause = sem_map[target];
    safe_assert(pause != NULL);
    safe_assert(Originals::sem_post(pause) == OPERATION_SUCCESSFUL);
}

void ExecutionTracker::pauseThread(thrID target) {
    glock();
    sem_t * pause = sem_map[target];
    safe_assert(pause != NULL);
    log->pauseThread(target);
    gunlock();
    int result;
    do {
        result = Originals::sem_wait(pause);
    } while (result == -1 && errno == EINTR);
    safe_assert(result == OPERATION_SUCCESSFUL);
}

void ExecutionTracker::glock() {
    Originals::pthread_mutex_lock(global_lock);
    lock_count++;
    
}

void ExecutionTracker::gunlock() {
    safe_assert(Originals::pthread_mutex_trylock(global_lock) != 
            OPERATION_SUCCESSFUL);
    lock_count--;
    Originals::pthread_mutex_unlock(global_lock);
}

void ExecutionTracker::disableThread(thrID target) {
    enable_map[target] = false;
}

void ExecutionTracker::enableThread(thrID target) {
    enable_map[target] = true;
}

void ExecutionTracker::enableJoiningThread(thrID myself) {
    if (join_map[myself] != UNJOINED) {
        enableThread(join_map[myself]);
    }
}


void ExecutionTracker::finalizeThread(thrID myself, void * addr) {
    disableThread(myself);
    finish_map[myself] = true;
    enableJoiningThread(myself);
    handleThreadFinish(myself);
    SchedPointInfo tmp(addr, "Thread Finish", 
            myself, enable_map, IS_NOT_YIELD);
    schedule(&tmp);
}

void ExecutionTracker::initializeThreadData(thrID target) {
    sem_t * tmp = new sem_t;
    safe_assert(Originals::sem_init(tmp, 0, 0) == OPERATION_SUCCESSFUL);
    sem_map[target] = tmp;

    tmp = new sem_t;
    safe_assert(Originals::sem_init(tmp, 0, 0) == OPERATION_SUCCESSFUL);
    wait_sem_map[target] = tmp;

    enableThread(target);

    finish_map[target] = false;

    exit_map[target] = false;

    join_map[target] = UNJOINED;

    lock_wait_map[target] = NULL;

    lock_ready_map[target] = NULL;

    sem_wait_map[target] = NULL;

    sem_ready_map[target] = NULL;

    cond_map[target] = NULL;

    wait_ready_map[target] = true;
    
    was_signalled_map[target] = true;
}

void ExecutionTracker::deadlockCheck() {
    map<thrID, bool>::iterator itr;
    for (itr = enable_map.begin(); itr != enable_map.end(); itr++) {
        if (itr->second) {
            return;
        }
    }

    log->programDeadlock();
    destructorHelper();
    _Exit(UNRECOVERABLE_ERROR);
}

thrID ExecutionTracker::pickNextSchedulingChoice(SchedPointInfo * s) {
    map<thrID, bool>::iterator itr;
    for (itr = enable_map.begin(); itr != enable_map.end(); itr++) {
        if (itr->second) {
            return itr->first;
        }
    }
    return INVALID_THREAD;
}

void ExecutionTracker::launchThread(thrID target, SchedPointInfo * s) {
    log->scheduling(target, s);
    safe_assert(isEnabled(target));
    if (cond_map[target] != NULL) {
        was_signalled_map[target] = false;
        waitWakeThread(target);
    } else {
        wakeThread(target);
    }
}


bool ExecutionTracker::BeforeMutexattrInit(thrID myself, 
        void* ret_addr, pthread_mutexattr_t* mattr) {
    return true;

}

/* used for "strict" schedulers */
void ExecutionTracker::compareScheduleSynchronization(string s1, string s2) {
    return;
}

void ExecutionTracker::recordScheduleInfo(thrID myself,
        thrID scheduled,
        string type,
        void * ret_addr) {

    string cs_type = "Not_Context_Switch";
    if (scheduled != myself) {
        contextswitch++;
        if (enable_map[myself]) {
            preemptions++;
            cs_type = "Preemption";
        } else {
            nonpreemptions++;
            cs_type = "NonPreemptive_Context_Switch";
        }
    }
    
    if (log->getPrint()){
        printf("Thread %d ", myself);
        printf("is scheduling at a %s ", type.c_str());
        printf("(addr: %p) ", ret_addr);
        printf("(scheduling: %d)\n", scheduled);
        printf("%s(from, to, id_addr)", cs_type.c_str());
        printf(":%d:%d:%p\n", myself, scheduled, ret_addr);
    }
}

void ExecutionTracker::followingChoiceFromLog(SchedPointInfo * s) {
    return;
}

thrID ExecutionTracker::schedule(SchedPointInfo * s) {
    void * ret_addr = s->ret_addr;
    string type = s->type;
    thrID myself = s->thread;
    map<thrID, bool> en_map = s->enabled;
    thrID nextchoice = log->getNextScheduleChoice(s);
    if (nextchoice == INVALID_THREAD) {
        s->fromLog = false;
        nextchoice = this->pickNextSchedulingChoice(s);
        if (nextchoice == INVALID_THREAD) {
            deadlockCheck();    
            bool notThrilleInternalError = false;
            safe_assert(notThrilleInternalError);
        }
        recordScheduleInfo(myself, nextchoice, type, ret_addr);
    } else {
        followingChoiceFromLog(s);
        s->fromLog = true;
        char buffer[MAX_READ_SIZE];
        sprintf(buffer, "%p", ret_addr);
        string tmp(buffer);
        if (ret_addr < (void*) 0x999999) {
            compareScheduleSynchronization(tmp, log->getLastID());
        } else {
            // heuristically skipping addr comparison 
            // because statement id looks like it lives in a dynamic library
            skipped_comparisons++;
        }
        recordScheduleInfo(myself, nextchoice, type, ret_addr);
    }
    s->enabled = enable_map;
    launchThread(nextchoice, s);
    return nextchoice; 
}

void ExecutionTracker::addThreadReadyOnSemaphore(thrID myself, sem_t * sem) {
    sem_ready_map[myself] = sem;
}

void ExecutionTracker::addThreadWaitingOnSemaphore(thrID myself, sem_t * sem) {
    sem_wait_map[myself] = sem;
}

void ExecutionTracker::determineSemaphoreStatus(thrID myself, sem_t * sem) {
    int val = checkSemaphoreValue(sem);
    if (val > 0) {
        enableThread(myself);
        addThreadReadyOnSemaphore(myself, sem);
    } else {
        disableThread(myself);
        addThreadWaitingOnSemaphore(myself, sem);
    }
}

bool ExecutionTracker::BeforeSemWait(thrID myself, void * ret_addr,
        sem_t * sem) {
    glock();
    log->BeforeSemWait(myself);
    safe_assert(sem != NULL);
    checkSemValidity(sem);
    determineSemaphoreStatus(myself, sem);
    SchedPointInfo tmp(ret_addr, "Before Sem Wait", 
            myself, enable_map, IS_NOT_YIELD, sem);
    schedule(&tmp);
    gunlock();
    pauseThread(myself);
    return false;
}

int ExecutionTracker::checkSemaphoreValue(sem_t * sem) {
    int a = -1;
    int * semval = &a;
    safe_assert(Originals::sem_getvalue(sem, semval) == OPERATION_SUCCESSFUL);
    return a; 
}

void ExecutionTracker::threadHasPassedSemaphore(thrID myself, sem_t * sem) {
    safe_assert(sem_ready_map[myself] == sem);
    sem_ready_map[myself] = NULL;
    int val = checkSemaphoreValue(sem);
    if (val <= 0) {
        map<thrID, sem_t *>::iterator itr;
        for (itr = sem_ready_map.begin(); itr != sem_ready_map.end(); itr++) {
            if (itr->second == sem) {
                sem_ready_map[itr->first] = NULL;
                sem_wait_map[itr->first] = sem;
                disableThread(itr->first);
            }
        }
    }
}

int ExecutionTracker::SimulateSemWait(thrID myself, void * ret_addr,
        sem_t * sem) {
    glock();
    safe_assert(checkSemaphoreValue(sem) > 0);
    log->SimulateSemWait(myself);
    checkSemValidity(sem);
    int result = Originals::sem_wait(sem);
    threadHasPassedSemaphore(myself, sem);
    gunlock();
    return result;
}

bool ExecutionTracker::BeforeSemPost(thrID myself, void * ret_addr,
        sem_t * sem) {
    return false;
}


void ExecutionTracker::semaphoreIsPosted(thrID myself, sem_t * sem) {
    map<thrID, sem_t *>::iterator itr;
    for (itr = sem_wait_map.begin(); itr != sem_wait_map.end(); itr++) {
        if (itr->second == sem) {
            enableThread(itr->first);
            sem_ready_map[itr->first] = sem;
            sem_wait_map[itr->first] = NULL;
        }
    }

}

int ExecutionTracker::SimulateSemPost(thrID myself, void * ret_addr,
        sem_t * sem) {
    glock();
    log->SimulateSemPost(myself);
    checkSemValidity(sem);
    int result = Originals::sem_post(sem);
    semaphoreIsPosted(myself, sem);
    SchedPointInfo tmp(ret_addr, "Simulate Sem Post", myself,
            enable_map, IS_NOT_YIELD, sem);
    schedule(&tmp);
    gunlock();
    pauseThread(myself);
    return result;
}

bool ExecutionTracker::BeforeSemInit(thrID myself, void * ret_addr,
        sem_t * sem, int pshared, unsigned int value) {
    semIsValid(sem);
    return true;
}



/* TODO: implement this */
bool ExecutionTracker::BeforeSigwait(thrID myself, void * ret_addr,
        const sigset_t * set, int * sig) {
    glock();
    disableThread(myself);
    SchedPointInfo tmp(ret_addr, "Before Sig Wait", myself, enable_map, false);
    schedule(&tmp);
    gunlock();
    bool sigwait_is_implemented = false;
    safe_assert(sigwait_is_implemented);
    return false;
}


bool ExecutionTracker::BeforeSetcanceltype(thrID myself, void * retaddr,
        int, int*) { 
    return true;
}


bool ExecutionTracker::BeforeBarrierInit(thrID myself, void * ret_addr,
        pthread_barrier_t * barrier, const pthread_barrierattr_t * battr,
        unsigned count) {
    glock();
    log->BeforeBarrierInit(myself);
    bt->initBarrier(barrier, count);
    barrierIsValid(barrier);
    gunlock();
    
    return true;
}

bool ExecutionTracker::BeforeBarrierWait(thrID myself, void * ret_addr,
        pthread_barrier_t * barrier) {
    glock();
    log->BeforeBarrierWait(myself);
    checkBarrierValidity(barrier);
    int target_count = bt->getBarrierCount(barrier);
    int threads_on_barrier = bt->getNumberThreadsWaitingOnBarrier(barrier);
    if (threads_on_barrier + 1 >= target_count) {
        vector<thrID> my_thr = bt->getThreadsWaitingOnBarrier(barrier);
        safe_assert(((unsigned) (my_thr.size() + 1)) 
                == (bt->getBarrierCount(barrier)));
        vector<thrID>::iterator itr;
        for (itr = my_thr.begin(); itr != my_thr.end(); itr++) {
            safe_assert(isDisabled((*itr)));
            enableThread((*itr));
        }
        enableThread(myself);
        bt->clearThreadsWaitingOnBarrier(barrier);
    } else {
        bt->threadWaitingOnBarrier(myself, barrier);
        disableThread(myself);
    }
    SchedPointInfo tmp(ret_addr, "Before Barrier Wait", myself,
            enable_map, IS_NOT_YIELD, barrier);
    schedule(&tmp);
    gunlock();
    pauseThread(myself);
    return false;
}



bool ExecutionTracker::isEnabled(thrID target) {
    if (target == INVALID_THREAD) {
        return false;
    }
    return enable_map[target];
}

bool ExecutionTracker::isDisabled(thrID target) {
    return (! isEnabled(target)); 
}


void ExecutionTracker::handleAfterCreate(thrID, void *) {

}

void ExecutionTracker::handleBeforeJoin(thrID, thrID, void *) {

}

void ExecutionTracker::handleBeforeMutexLock(thrID, void *, pthread_mutex_t *,
        bool isWait) {

}

void ExecutionTracker::handleAfterMutexUnlock(thrID, void *, pthread_mutex_t *,
        bool isWait) {

}

void ExecutionTracker::handleSimulateCondWait(thrID, void *, 
                pthread_cond_t *, 
                pthread_mutex_t *) {

}

void ExecutionTracker::handleSimulateCondTimedwait(thrID, void *, 
                pthread_cond_t *, 
                pthread_mutex_t *) {

}


void ExecutionTracker::handleSimulateCondSignal(thrID, void *,
        pthread_cond_t *) {

}

void ExecutionTracker::handleSimulateCondBroadcast(thrID, void *,
        pthread_cond_t *) {

}

void ExecutionTracker::handleMyMemoryRead(thrID, void *, void *) {

}

void ExecutionTracker::handleMyMemoryWrite(thrID, void *, void *) {

}

void ExecutionTracker::handleThreadStart(thrID, void * (*) (void *)) {

}

void ExecutionTracker::handleThreadFinish(thrID) {

}

void ExecutionTracker::handleMyUsleep(thrID, void *) {

}

void ExecutionTracker::handleMySleep(thrID, void *) {

}


