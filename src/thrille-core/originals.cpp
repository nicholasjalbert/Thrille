#include "originals.h"

#define init_original(f, type) { \
    _##f = (type) dlsym(RTLD_NEXT, #f); \
    if (_##f == NULL) { \
        printf("Thrille: originals %s init fail\n", #f); \
    }\
}

volatile bool Originals::_initialized = false;

int (* volatile Originals::_sem_wait) (sem_t *) = NULL;
int (* volatile Originals::_pthread_mutex_lock) (pthread_mutex_t *) = NULL;
unsigned int (* volatile Originals::_sleep) (unsigned int) = NULL;
int (* volatile Originals::_pthread_condattr_destroy) (pthread_condattr_t *) = NULL;
int (* volatile Originals::_pthread_mutexattr_getpshared) (const pthread_mutexattr_t *, int *) = NULL;
int (* volatile Originals::_pthread_setconcurrency) (int) = NULL;
int (* volatile Originals::_pthread_getconcurrency) () = NULL;
int (* volatile Originals::_sem_destroy) (sem_t *) = NULL;
void (* volatile Originals::_pthread_testcancel) () = NULL;
int (* volatile Originals::_pthread_attr_getscope) (const pthread_attr_t *, int *) = NULL;
int (* volatile Originals::_pthread_mutex_init) (pthread_mutex_t *, const pthread_mutexattr_t *) = NULL;
int (* volatile Originals::_sem_init) (sem_t *, int, unsigned int) = NULL;
int (* volatile Originals::_pthread_mutex_trylock) (pthread_mutex_t *) = NULL;
int (* volatile Originals::_pthread_rwlockattr_init) (pthread_rwlockattr_t *) = NULL;
int (* volatile Originals::_pthread_mutexattr_setpshared) (pthread_mutexattr_t *, int) = NULL;
int (* volatile Originals::_pthread_rwlockattr_destroy) (pthread_rwlockattr_t *) = NULL;
int (* volatile Originals::_pthread_barrier_destroy) (pthread_barrier_t *) = NULL;
int (* volatile Originals::_pthread_getschedparam) (pthread_t, int *, struct sched_param *) = NULL;
int (* volatile Originals::_pthread_attr_getschedparam) (const pthread_attr_t *, struct sched_param *) = NULL;
int (* volatile Originals::_pthread_cancel) (pthread_t) = NULL;
int (* volatile Originals::_pthread_cond_wait) (pthread_cond_t *, pthread_mutex_t *) = NULL;
int (* volatile Originals::_pthread_condattr_getpshared) (const pthread_condattr_t *, int *) = NULL;
int (* volatile Originals::_pthread_spin_trylock) (pthread_spinlock_t *) = NULL;
int (* volatile Originals::_pthread_attr_getinheritsched) (const pthread_attr_t *, int *) = NULL;
int (* volatile Originals::_pthread_attr_setstack) (pthread_attr_t *, void *, size_t) = NULL;
int (* volatile Originals::_pthread_cond_signal) (pthread_cond_t *) = NULL;
int (* volatile Originals::_sem_getvalue) (sem_t *, int *) = NULL;
int (* volatile Originals::_pthread_mutexattr_init) (pthread_mutexattr_t *) = NULL;
int (* volatile Originals::_pthread_barrier_wait) (pthread_barrier_t *) = NULL;
int (* volatile Originals::_pthread_spin_destroy) (pthread_spinlock_t *) = NULL;
int (* volatile Originals::_pthread_key_delete) (pthread_key_t) = NULL;
int (* volatile Originals::_pthread_setspecific) (pthread_key_t, const void *) = NULL;
int (* volatile Originals::_pthread_attr_setstacksize) (pthread_attr_t *, size_t) = NULL;
int (* volatile Originals::_pthread_setcancelstate) (int, int *) = NULL;
int (* volatile Originals::_pthread_barrierattr_setpshared) (pthread_barrierattr_t *, int) = NULL;
int (* volatile Originals::_pthread_rwlock_timedrdlock) (pthread_rwlock_t *, const struct timespec *) = NULL;
int (* volatile Originals::_pthread_detach) (pthread_t) = NULL;
int (* volatile Originals::_usleep) (useconds_t) = NULL;
int (* volatile Originals::_pthread_rwlock_destroy) (pthread_rwlock_t *) = NULL;
void (* volatile Originals::_pthread_exit) (void *) = NULL;
int (* volatile Originals::_pthread_attr_setdetachstate) (pthread_attr_t *, int) = NULL;
int (* volatile Originals::_pthread_rwlock_trywrlock) (pthread_rwlock_t *) = NULL;
int (* volatile Originals::_pthread_attr_init) (pthread_attr_t *) = NULL;
int (* volatile Originals::_pthread_attr_setguardsize) (pthread_attr_t *, size_t) = NULL;
int (* volatile Originals::_pthread_condattr_setpshared) (pthread_condattr_t *, int) = NULL;
int (* volatile Originals::_pthread_mutex_setprioceiling) (pthread_mutex_t *, int, int *) = NULL;
int (* volatile Originals::_pthread_mutexattr_getprotocol) (const pthread_mutexattr_t *, int *) = NULL;
int (* volatile Originals::_pthread_attr_getstack) (const pthread_attr_t *, void **, size_t *) = NULL;
int (* volatile Originals::_pthread_condattr_init) (pthread_condattr_t *) = NULL;
int (* volatile Originals::_pthread_mutexattr_gettype) (const pthread_mutexattr_t *, int *) = NULL;
int (* volatile Originals::_sem_post) (sem_t *) = NULL;
int (* volatile Originals::_pthread_attr_getstacksize) (const pthread_attr_t *, size_t *) = NULL;
int (* volatile Originals::_pthread_create) (pthread_t *, const pthread_attr_t *, void *(*)(void *), void *) = NULL;
int (* volatile Originals::_sem_timedwait) (sem_t *, const struct timespec *) = NULL;
int (* volatile Originals::_pthread_equal) (pthread_t, pthread_t) = NULL;
int (* volatile Originals::_pthread_spin_init) (pthread_spinlock_t *, int) = NULL;
int (* volatile Originals::_sched_yield) () = NULL;
int (* volatile Originals::_pthread_key_create) (pthread_key_t *, void (*)(void *)) = NULL;
int (* volatile Originals::_pthread_mutexattr_destroy) (pthread_mutexattr_t *) = NULL;
int (* volatile Originals::_pthread_cond_timedwait) (pthread_cond_t *, pthread_mutex_t *, const struct timespec *) = NULL;
int (* volatile Originals::_pthread_condattr_getclock) (const pthread_condattr_t *, clockid_t *) = NULL;
int (* volatile Originals::_pthread_setschedparam) (pthread_t, int, const struct sched_param *) = NULL;
int (* volatile Originals::_pthread_attr_setinheritsched) (pthread_attr_t *, int) = NULL;
int (* volatile Originals::_pthread_mutex_unlock) (pthread_mutex_t *) = NULL;
int (* volatile Originals::_pthread_attr_setschedparam) (pthread_attr_t *, const struct sched_param *) = NULL;
int (* volatile Originals::_pthread_rwlockattr_setpshared) (pthread_rwlockattr_t *, int) = NULL;
int (* volatile Originals::_pthread_attr_setstackaddr) (pthread_attr_t *, void *) = NULL;
int (* volatile Originals::_pthread_mutex_destroy) (pthread_mutex_t *) = NULL;
int (* volatile Originals::_pthread_attr_getschedpolicy) (const pthread_attr_t *, int *) = NULL;
int (* volatile Originals::_pthread_setcanceltype) (int, int *) = NULL;
int (* volatile Originals::_sigwait) (const sigset_t *, int *) = NULL;
int (* volatile Originals::_pthread_rwlock_init) (pthread_rwlock_t *, const pthread_rwlockattr_t *) = NULL;
int (* volatile Originals::_pthread_rwlock_unlock) (pthread_rwlock_t *) = NULL;
int (* volatile Originals::_pthread_mutexattr_setprotocol) (pthread_mutexattr_t *, int) = NULL;
int (* volatile Originals::_pthread_mutex_getprioceiling) (const pthread_mutex_t *, int *) = NULL;
int (* volatile Originals::_sem_trywait) (sem_t *) = NULL;
int (* volatile Originals::_pthread_attr_getstackaddr) (const pthread_attr_t *, void **) = NULL;
int (* volatile Originals::_pthread_mutexattr_settype) (pthread_mutexattr_t *, int) = NULL;
int (* volatile Originals::_pthread_getcpuclockid) (pthread_t, clockid_t *) = NULL;
int (* volatile Originals::_pthread_cond_destroy) (pthread_cond_t *) = NULL;
int (* volatile Originals::_pthread_spin_lock) (pthread_spinlock_t *) = NULL;
int (* volatile Originals::_pthread_once) (pthread_once_t *, void (*)(void)) = NULL;
int (* volatile Originals::_pthread_atfork) (void (*)(void), void (*)(void), void(*)(void)) = NULL;
int (* volatile Originals::_pthread_attr_setscope) (pthread_attr_t *, int) = NULL;
void * (* volatile Originals::_pthread_getspecific) (pthread_key_t) = NULL;
int (* volatile Originals::_pthread_attr_destroy) (pthread_attr_t *) = NULL;
int (* volatile Originals::_pthread_mutex_timedlock) (pthread_mutex_t *, const struct timespec *) = NULL;
int (* volatile Originals::_pthread_barrier_init) (pthread_barrier_t *, const pthread_barrierattr_t *, unsigned) = NULL;
int (* volatile Originals::_pthread_setschedprio) (pthread_t, int) = NULL;
int (* volatile Originals::_pthread_condattr_setclock) (pthread_condattr_t *, clockid_t) = NULL;
int (* volatile Originals::_pthread_cond_broadcast) (pthread_cond_t *) = NULL;
int (* volatile Originals::_pthread_spin_unlock) (pthread_spinlock_t *) = NULL;
int (* volatile Originals::_pthread_rwlock_rdlock) (pthread_rwlock_t *) = NULL;
int (* volatile Originals::_pthread_rwlock_wrlock) (pthread_rwlock_t *) = NULL;
int (* volatile Originals::_pthread_join) (pthread_t, void **) = NULL;
int (* volatile Originals::_pthread_cond_init) (pthread_cond_t *, const pthread_condattr_t *) = NULL;
int (* volatile Originals::_pthread_attr_getguardsize) (const pthread_attr_t *, size_t *) = NULL;
int (* volatile Originals::_pthread_barrierattr_init) (pthread_barrierattr_t *) = NULL;
int (* volatile Originals::_pthread_rwlock_tryrdlock) (pthread_rwlock_t *) = NULL;
int (* volatile Originals::_pthread_attr_setschedpolicy) (pthread_attr_t *, int) = NULL;
int (* volatile Originals::_pthread_barrierattr_destroy) (pthread_barrierattr_t *) = NULL;
pthread_t (* volatile Originals::_pthread_self) () = NULL;
int (* volatile Originals::_pthread_attr_getdetachstate) (const pthread_attr_t *, int *) = NULL;
int (* volatile Originals::_pthread_rwlock_timedwrlock) (pthread_rwlock_t *, const struct timespec *) = NULL;
int (* volatile Originals::_pthread_mutexattr_setprioceiling) (pthread_mutexattr_t *, int) = NULL;

volatile bool Originals::is_initialized()
{
    return _initialized;
}

void Originals::_initialize()
{
    safe_assert(!_initialized);

    init_original(sem_wait, int (* volatile) (sem_t *));
    init_original(pthread_mutex_lock, int (* volatile) (pthread_mutex_t *));
    init_original(sleep, unsigned int (* volatile) (unsigned int));
    init_original(pthread_condattr_destroy, int (* volatile) (pthread_condattr_t *));
    init_original(pthread_mutexattr_getpshared, int (* volatile) (const pthread_mutexattr_t *, int *));
    init_original(pthread_setconcurrency, int (* volatile) (int));
    init_original(pthread_getconcurrency, int (* volatile) ());
    init_original(sem_destroy, int (* volatile) (sem_t *));
    init_original(pthread_testcancel, void (* volatile) ());
    init_original(pthread_attr_getscope, int (* volatile) (const pthread_attr_t *, int *));
    init_original(pthread_mutex_init, int (* volatile) (pthread_mutex_t *, const pthread_mutexattr_t *));
    init_original(sem_init, int (* volatile) (sem_t *, int, unsigned int));
    init_original(pthread_mutex_trylock, int (* volatile) (pthread_mutex_t *));
    init_original(pthread_rwlockattr_init, int (* volatile) (pthread_rwlockattr_t *));
    init_original(pthread_mutexattr_setpshared, int (* volatile) (pthread_mutexattr_t *, int));
    init_original(pthread_rwlockattr_destroy, int (* volatile) (pthread_rwlockattr_t *));
    init_original(pthread_barrier_destroy, int (* volatile) (pthread_barrier_t *));
    init_original(pthread_getschedparam, int (* volatile) (pthread_t, int *, struct sched_param *));
    init_original(pthread_attr_getschedparam, int (* volatile) (const pthread_attr_t *, struct sched_param *));
    init_original(pthread_cancel, int (* volatile) (pthread_t));
    init_original(pthread_cond_wait, int (* volatile) (pthread_cond_t *, pthread_mutex_t *));
    init_original(pthread_condattr_getpshared, int (* volatile) (const pthread_condattr_t *, int *));
    init_original(pthread_spin_trylock, int (* volatile) (pthread_spinlock_t *));
    init_original(pthread_attr_getinheritsched, int (* volatile) (const pthread_attr_t *, int *));
    init_original(pthread_attr_setstack, int (* volatile) (pthread_attr_t *, void *, size_t));
    init_original(pthread_cond_signal, int (* volatile) (pthread_cond_t *));
    init_original(sem_getvalue, int (* volatile) (sem_t *, int *));
    init_original(pthread_mutexattr_init, int (* volatile) (pthread_mutexattr_t *));
    init_original(pthread_barrier_wait, int (* volatile) (pthread_barrier_t *));
    init_original(pthread_spin_destroy, int (* volatile) (pthread_spinlock_t *));
    init_original(pthread_key_delete, int (* volatile) (pthread_key_t));
    init_original(pthread_setspecific, int (* volatile) (pthread_key_t, const void *));
    init_original(pthread_attr_setstacksize, int (* volatile) (pthread_attr_t *, size_t));
    init_original(pthread_setcancelstate, int (* volatile) (int, int *));
    init_original(pthread_barrierattr_setpshared, int (* volatile) (pthread_barrierattr_t *, int));
    init_original(pthread_rwlock_timedrdlock, int (* volatile) (pthread_rwlock_t *, const struct timespec *));
    init_original(pthread_detach, int (* volatile) (pthread_t));
    init_original(usleep, int (* volatile) (useconds_t));
    init_original(pthread_rwlock_destroy, int (* volatile) (pthread_rwlock_t *));
    init_original(pthread_exit, void (* volatile) (void *));
    init_original(pthread_attr_setdetachstate, int (* volatile) (pthread_attr_t *, int));
    init_original(pthread_rwlock_trywrlock, int (* volatile) (pthread_rwlock_t *));
    init_original(pthread_attr_init, int (* volatile) (pthread_attr_t *));
    init_original(pthread_attr_setguardsize, int (* volatile) (pthread_attr_t *, size_t));
    init_original(pthread_condattr_setpshared, int (* volatile) (pthread_condattr_t *, int));
    init_original(pthread_mutex_setprioceiling, int (* volatile) (pthread_mutex_t *, int, int *));
    init_original(pthread_mutexattr_getprotocol, int (* volatile) (const pthread_mutexattr_t *, int *));
    init_original(pthread_attr_getstack, int (* volatile) (const pthread_attr_t *, void **, size_t *));
    init_original(pthread_condattr_init, int (* volatile) (pthread_condattr_t *));
    init_original(pthread_mutexattr_gettype, int (* volatile) (const pthread_mutexattr_t *, int *));
    init_original(sem_post, int (* volatile) (sem_t *));
    init_original(pthread_attr_getstacksize, int (* volatile) (const pthread_attr_t *, size_t *));
    init_original(pthread_create, int (* volatile) (pthread_t *, const pthread_attr_t *, void *(*)(void *), void *));
    init_original(sem_timedwait, int (* volatile) (sem_t *, const struct timespec *));
    init_original(pthread_equal, int (* volatile) (pthread_t, pthread_t));
    init_original(pthread_spin_init, int (* volatile) (pthread_spinlock_t *, int));
    init_original(sched_yield, int (* volatile) ());
    init_original(pthread_key_create, int (* volatile) (pthread_key_t *, void (*)(void *)));
    init_original(pthread_mutexattr_destroy, int (* volatile) (pthread_mutexattr_t *));
    init_original(pthread_cond_timedwait, int (* volatile) (pthread_cond_t *, pthread_mutex_t *, const struct timespec *));
    init_original(pthread_condattr_getclock, int (* volatile) (const pthread_condattr_t *, clockid_t *));
    init_original(pthread_setschedparam, int (* volatile) (pthread_t, int, const struct sched_param *));
    init_original(pthread_attr_setinheritsched, int (* volatile) (pthread_attr_t *, int));
    init_original(pthread_mutex_unlock, int (* volatile) (pthread_mutex_t *));
    init_original(pthread_attr_setschedparam, int (* volatile) (pthread_attr_t *, const struct sched_param *));
    init_original(pthread_rwlockattr_setpshared, int (* volatile) (pthread_rwlockattr_t *, int));
    init_original(pthread_attr_setstackaddr, int (* volatile) (pthread_attr_t *, void *));
    init_original(pthread_mutex_destroy, int (* volatile) (pthread_mutex_t *));
    init_original(pthread_attr_getschedpolicy, int (* volatile) (const pthread_attr_t *, int *));
    init_original(pthread_setcanceltype, int (* volatile) (int, int *));
    init_original(sigwait, int (* volatile) (const sigset_t *, int *));
    init_original(pthread_rwlock_init, int (* volatile) (pthread_rwlock_t *, const pthread_rwlockattr_t *));
    init_original(pthread_rwlock_unlock, int (* volatile) (pthread_rwlock_t *));
    init_original(pthread_mutexattr_setprotocol, int (* volatile) (pthread_mutexattr_t *, int));
    init_original(pthread_mutex_getprioceiling, int (* volatile) (const pthread_mutex_t *, int *));
    init_original(sem_trywait, int (* volatile) (sem_t *));
    init_original(pthread_attr_getstackaddr, int (* volatile) (const pthread_attr_t *, void **));
    init_original(pthread_mutexattr_settype, int (* volatile) (pthread_mutexattr_t *, int));
    init_original(pthread_getcpuclockid, int (* volatile) (pthread_t, clockid_t *));
    init_original(pthread_cond_destroy, int (* volatile) (pthread_cond_t *));
    init_original(pthread_spin_lock, int (* volatile) (pthread_spinlock_t *));
    init_original(pthread_once, int (* volatile) (pthread_once_t *, void (*)(void)));
    init_original(pthread_atfork, int (* volatile) (void (*)(void), void (*)(void), void(*)(void)));
    init_original(pthread_attr_setscope, int (* volatile) (pthread_attr_t *, int));
    init_original(pthread_getspecific, void * (* volatile) (pthread_key_t));
    init_original(pthread_attr_destroy, int (* volatile) (pthread_attr_t *));
    init_original(pthread_mutex_timedlock, int (* volatile) (pthread_mutex_t *, const struct timespec *));
    init_original(pthread_barrier_init, int (* volatile) (pthread_barrier_t *, const pthread_barrierattr_t *, unsigned));
    init_original(pthread_setschedprio, int (* volatile) (pthread_t, int));
    init_original(pthread_condattr_setclock, int (* volatile) (pthread_condattr_t *, clockid_t));
    init_original(pthread_cond_broadcast, int (* volatile) (pthread_cond_t *));
    init_original(pthread_spin_unlock, int (* volatile) (pthread_spinlock_t *));
    init_original(pthread_rwlock_rdlock, int (* volatile) (pthread_rwlock_t *));
    init_original(pthread_rwlock_wrlock, int (* volatile) (pthread_rwlock_t *));
    init_original(pthread_join, int (* volatile) (pthread_t, void **));
    init_original(pthread_cond_init, int (* volatile) (pthread_cond_t *, const pthread_condattr_t *));
    init_original(pthread_attr_getguardsize, int (* volatile) (const pthread_attr_t *, size_t *));
    init_original(pthread_barrierattr_init, int (* volatile) (pthread_barrierattr_t *));
    init_original(pthread_rwlock_tryrdlock, int (* volatile) (pthread_rwlock_t *));
    init_original(pthread_attr_setschedpolicy, int (* volatile) (pthread_attr_t *, int));
    init_original(pthread_barrierattr_destroy, int (* volatile) (pthread_barrierattr_t *));
    init_original(pthread_self, pthread_t (* volatile) ());
    init_original(pthread_attr_getdetachstate, int (* volatile) (const pthread_attr_t *, int *));
    init_original(pthread_rwlock_timedwrlock, int (* volatile) (pthread_rwlock_t *, const struct timespec *));
    init_original(pthread_mutexattr_setprioceiling, int (* volatile) (pthread_mutexattr_t *, int));
    
    _initialized = true;
}


void Originals::initialize()
{
    safe_assert(!_initialized);

    _initialize();

    safe_assert(_initialized);
}

int Originals::sem_wait(sem_t * param0) {
    if (_sem_wait== NULL) {
        printf("Thrille: ERROR: original sem_wait is NULL\n");
        return 0;
    }
return _sem_wait(param0);
}

int Originals::pthread_mutex_lock(pthread_mutex_t * param0) {
    if (_pthread_mutex_lock== NULL) {
        printf("Thrille: ERROR: original pthread_mutex_lock is NULL\n");
        return 0;
    }
return _pthread_mutex_lock(param0);
}

unsigned int Originals::sleep(unsigned int param0) {
    if (_sleep== NULL) {
        printf("Thrille: ERROR: original sleep is NULL\n");
        return 0;
    }
return _sleep(param0);
}

int Originals::pthread_condattr_destroy(pthread_condattr_t * param0) {
    if (_pthread_condattr_destroy== NULL) {
        printf("Thrille: ERROR: original pthread_condattr_destroy is NULL\n");
        return 0;
    }
return _pthread_condattr_destroy(param0);
}

int Originals::pthread_mutexattr_getpshared(const pthread_mutexattr_t * param0, int * param1) {
    if (_pthread_mutexattr_getpshared== NULL) {
        printf("Thrille: ERROR: original pthread_mutexattr_getpshared is NULL\n");
        return 0;
    }
return _pthread_mutexattr_getpshared(param0, param1);
}

int Originals::pthread_setconcurrency(int param0) {
    if (_pthread_setconcurrency== NULL) {
        printf("Thrille: ERROR: original pthread_setconcurrency is NULL\n");
        return 0;
    }
return _pthread_setconcurrency(param0);
}

int Originals::pthread_getconcurrency() {
    if (_pthread_getconcurrency== NULL) {
        printf("Thrille: ERROR: original pthread_getconcurrency is NULL\n");
        return 0;
    }
return _pthread_getconcurrency();
}

int Originals::sem_destroy(sem_t * param0) {
    if (_sem_destroy== NULL) {
        printf("Thrille: ERROR: original sem_destroy is NULL\n");
        return 0;
    }
return _sem_destroy(param0);
}

void Originals::pthread_testcancel() {
    if (_pthread_testcancel== NULL) {
        printf("Thrille: ERROR: original pthread_testcancel is NULL\n");
        return;;
    }
return _pthread_testcancel();
}

int Originals::pthread_attr_getscope(const pthread_attr_t * param0, int * param1) {
    if (_pthread_attr_getscope== NULL) {
        printf("Thrille: ERROR: original pthread_attr_getscope is NULL\n");
        return 0;
    }
return _pthread_attr_getscope(param0, param1);
}

int Originals::pthread_mutex_init(pthread_mutex_t * param0, const pthread_mutexattr_t * param1) {
    if (_pthread_mutex_init== NULL) {
        printf("Thrille: ERROR: original pthread_mutex_init is NULL\n");
        return 0;
    }
return _pthread_mutex_init(param0, param1);
}

int Originals::sem_init(sem_t * param0, int param1, unsigned int param2) {
    if (_sem_init== NULL) {
        printf("Thrille: ERROR: original sem_init is NULL\n");
        return 0;
    }
return _sem_init(param0, param1, param2);
}

int Originals::pthread_mutex_trylock(pthread_mutex_t * param0) {
    if (_pthread_mutex_trylock== NULL) {
        printf("Thrille: ERROR: original pthread_mutex_trylock is NULL\n");
        return 0;
    }
return _pthread_mutex_trylock(param0);
}

int Originals::pthread_rwlockattr_init(pthread_rwlockattr_t * param0) {
    if (_pthread_rwlockattr_init== NULL) {
        printf("Thrille: ERROR: original pthread_rwlockattr_init is NULL\n");
        return 0;
    }
return _pthread_rwlockattr_init(param0);
}

int Originals::pthread_mutexattr_setpshared(pthread_mutexattr_t * param0, int param1) {
    if (_pthread_mutexattr_setpshared== NULL) {
        printf("Thrille: ERROR: original pthread_mutexattr_setpshared is NULL\n");
        return 0;
    }
return _pthread_mutexattr_setpshared(param0, param1);
}

int Originals::pthread_rwlockattr_destroy(pthread_rwlockattr_t * param0) {
    if (_pthread_rwlockattr_destroy== NULL) {
        printf("Thrille: ERROR: original pthread_rwlockattr_destroy is NULL\n");
        return 0;
    }
return _pthread_rwlockattr_destroy(param0);
}

int Originals::pthread_barrier_destroy(pthread_barrier_t * param0) {
    if (_pthread_barrier_destroy== NULL) {
        printf("Thrille: ERROR: original pthread_barrier_destroy is NULL\n");
        return 0;
    }
return _pthread_barrier_destroy(param0);
}

int Originals::pthread_getschedparam(pthread_t param0, int * param1, struct sched_param * param2) {
    if (_pthread_getschedparam== NULL) {
        printf("Thrille: ERROR: original pthread_getschedparam is NULL\n");
        return 0;
    }
return _pthread_getschedparam(param0, param1, param2);
}

int Originals::pthread_attr_getschedparam(const pthread_attr_t * param0, struct sched_param * param1) {
    if (_pthread_attr_getschedparam== NULL) {
        printf("Thrille: ERROR: original pthread_attr_getschedparam is NULL\n");
        return 0;
    }
return _pthread_attr_getschedparam(param0, param1);
}

int Originals::pthread_cancel(pthread_t param0) {
    if (_pthread_cancel== NULL) {
        printf("Thrille: ERROR: original pthread_cancel is NULL\n");
        return 0;
    }
return _pthread_cancel(param0);
}

int Originals::pthread_cond_wait(pthread_cond_t * param0, pthread_mutex_t * param1) {
    if (_pthread_cond_wait== NULL) {
        printf("Thrille: ERROR: original pthread_cond_wait is NULL\n");
        return 0;
    }
return _pthread_cond_wait(param0, param1);
}

int Originals::pthread_condattr_getpshared(const pthread_condattr_t * param0, int * param1) {
    if (_pthread_condattr_getpshared== NULL) {
        printf("Thrille: ERROR: original pthread_condattr_getpshared is NULL\n");
        return 0;
    }
return _pthread_condattr_getpshared(param0, param1);
}

int Originals::pthread_spin_trylock(pthread_spinlock_t * param0) {
    if (_pthread_spin_trylock== NULL) {
        printf("Thrille: ERROR: original pthread_spin_trylock is NULL\n");
        return 0;
    }
return _pthread_spin_trylock(param0);
}

int Originals::pthread_attr_getinheritsched(const pthread_attr_t * param0, int * param1) {
    if (_pthread_attr_getinheritsched== NULL) {
        printf("Thrille: ERROR: original pthread_attr_getinheritsched is NULL\n");
        return 0;
    }
return _pthread_attr_getinheritsched(param0, param1);
}

int Originals::pthread_attr_setstack(pthread_attr_t * param0, void * param1, size_t param2) {
    if (_pthread_attr_setstack== NULL) {
        printf("Thrille: ERROR: original pthread_attr_setstack is NULL\n");
        return 0;
    }
return _pthread_attr_setstack(param0, param1, param2);
}

int Originals::pthread_cond_signal(pthread_cond_t * param0) {
    if (_pthread_cond_signal== NULL) {
        printf("Thrille: ERROR: original pthread_cond_signal is NULL\n");
        return 0;
    }
return _pthread_cond_signal(param0);
}

int Originals::sem_getvalue(sem_t * param0, int * param1) {
    if (_sem_getvalue== NULL) {
        printf("Thrille: ERROR: original sem_getvalue is NULL\n");
        return 0;
    }
return _sem_getvalue(param0, param1);
}

int Originals::pthread_mutexattr_init(pthread_mutexattr_t * param0) {
    if (_pthread_mutexattr_init== NULL) {
        printf("Thrille: ERROR: original pthread_mutexattr_init is NULL\n");
        return 0;
    }
return _pthread_mutexattr_init(param0);
}

int Originals::pthread_barrier_wait(pthread_barrier_t * param0) {
    if (_pthread_barrier_wait== NULL) {
        printf("Thrille: ERROR: original pthread_barrier_wait is NULL\n");
        return 0;
    }
return _pthread_barrier_wait(param0);
}

int Originals::pthread_spin_destroy(pthread_spinlock_t * param0) {
    if (_pthread_spin_destroy== NULL) {
        printf("Thrille: ERROR: original pthread_spin_destroy is NULL\n");
        return 0;
    }
return _pthread_spin_destroy(param0);
}

int Originals::pthread_key_delete(pthread_key_t param0) {
    if (_pthread_key_delete== NULL) {
        printf("Thrille: ERROR: original pthread_key_delete is NULL\n");
        return 0;
    }
return _pthread_key_delete(param0);
}

int Originals::pthread_setspecific(pthread_key_t param0, const void * param1) {
    if (_pthread_setspecific== NULL) {
        printf("Thrille: ERROR: original pthread_setspecific is NULL\n");
        return 0;
    }
return _pthread_setspecific(param0, param1);
}

int Originals::pthread_attr_setstacksize(pthread_attr_t * param0, size_t param1) {
    if (_pthread_attr_setstacksize== NULL) {
        printf("Thrille: ERROR: original pthread_attr_setstacksize is NULL\n");
        return 0;
    }
return _pthread_attr_setstacksize(param0, param1);
}

int Originals::pthread_setcancelstate(int param0, int * param1) {
    if (_pthread_setcancelstate== NULL) {
        printf("Thrille: ERROR: original pthread_setcancelstate is NULL\n");
        return 0;
    }
return _pthread_setcancelstate(param0, param1);
}

int Originals::pthread_barrierattr_setpshared(pthread_barrierattr_t * param0, int param1) {
    if (_pthread_barrierattr_setpshared== NULL) {
        printf("Thrille: ERROR: original pthread_barrierattr_setpshared is NULL\n");
        return 0;
    }
return _pthread_barrierattr_setpshared(param0, param1);
}

int Originals::pthread_rwlock_timedrdlock(pthread_rwlock_t * param0, const struct timespec * param1) {
    if (_pthread_rwlock_timedrdlock== NULL) {
        printf("Thrille: ERROR: original pthread_rwlock_timedrdlock is NULL\n");
        return 0;
    }
return _pthread_rwlock_timedrdlock(param0, param1);
}

int Originals::pthread_detach(pthread_t param0) {
    if (_pthread_detach== NULL) {
        printf("Thrille: ERROR: original pthread_detach is NULL\n");
        return 0;
    }
return _pthread_detach(param0);
}

int Originals::usleep(useconds_t param0) {
    if (_usleep== NULL) {
        printf("Thrille: ERROR: original usleep is NULL\n");
        return 0;
    }
return _usleep(param0);
}

int Originals::pthread_rwlock_destroy(pthread_rwlock_t * param0) {
    if (_pthread_rwlock_destroy== NULL) {
        printf("Thrille: ERROR: original pthread_rwlock_destroy is NULL\n");
        return 0;
    }
return _pthread_rwlock_destroy(param0);
}

void Originals::pthread_exit(void * param0) {
    if (_pthread_exit== NULL) {
        printf("Thrille: ERROR: original pthread_exit is NULL\n");
        return;;
    }
_pthread_exit(param0);
}

int Originals::pthread_attr_setdetachstate(pthread_attr_t * param0, int param1) {
    if (_pthread_attr_setdetachstate== NULL) {
        printf("Thrille: ERROR: original pthread_attr_setdetachstate is NULL\n");
        return 0;
    }
return _pthread_attr_setdetachstate(param0, param1);
}

int Originals::pthread_rwlock_trywrlock(pthread_rwlock_t * param0) {
    if (_pthread_rwlock_trywrlock== NULL) {
        printf("Thrille: ERROR: original pthread_rwlock_trywrlock is NULL\n");
        return 0;
    }
return _pthread_rwlock_trywrlock(param0);
}

int Originals::pthread_attr_init(pthread_attr_t * param0) {
    if (_pthread_attr_init== NULL) {
        printf("Thrille: ERROR: original pthread_attr_init is NULL\n");
        return 0;
    }
return _pthread_attr_init(param0);
}

int Originals::pthread_attr_setguardsize(pthread_attr_t * param0, size_t param1) {
    if (_pthread_attr_setguardsize== NULL) {
        printf("Thrille: ERROR: original pthread_attr_setguardsize is NULL\n");
        return 0;
    }
return _pthread_attr_setguardsize(param0, param1);
}

int Originals::pthread_condattr_setpshared(pthread_condattr_t * param0, int param1) {
    if (_pthread_condattr_setpshared== NULL) {
        printf("Thrille: ERROR: original pthread_condattr_setpshared is NULL\n");
        return 0;
    }
return _pthread_condattr_setpshared(param0, param1);
}

int Originals::pthread_mutex_setprioceiling(pthread_mutex_t * param0, int param1, int * param2) {
    if (_pthread_mutex_setprioceiling== NULL) {
        printf("Thrille: ERROR: original pthread_mutex_setprioceiling is NULL\n");
        return 0;
    }
return _pthread_mutex_setprioceiling(param0, param1, param2);
}

int Originals::pthread_mutexattr_getprotocol(const pthread_mutexattr_t * param0, int * param1) {
    if (_pthread_mutexattr_getprotocol== NULL) {
        printf("Thrille: ERROR: original pthread_mutexattr_getprotocol is NULL\n");
        return 0;
    }
return _pthread_mutexattr_getprotocol(param0, param1);
}

int Originals::pthread_attr_getstack(const pthread_attr_t * param0, void ** param1, size_t * param2) {
    if (_pthread_attr_getstack== NULL) {
        printf("Thrille: ERROR: original pthread_attr_getstack is NULL\n");
        return 0;
    }
return _pthread_attr_getstack(param0, param1, param2);
}

int Originals::pthread_condattr_init(pthread_condattr_t * param0) {
    if (_pthread_condattr_init== NULL) {
        printf("Thrille: ERROR: original pthread_condattr_init is NULL\n");
        return 0;
    }
return _pthread_condattr_init(param0);
}

int Originals::pthread_mutexattr_gettype(const pthread_mutexattr_t * param0, int * param1) {
    if (_pthread_mutexattr_gettype== NULL) {
        printf("Thrille: ERROR: original pthread_mutexattr_gettype is NULL\n");
        return 0;
    }
return _pthread_mutexattr_gettype(param0, param1);
}

int Originals::sem_post(sem_t * param0) {
    if (_sem_post== NULL) {
        printf("Thrille: ERROR: original sem_post is NULL\n");
        return 0;
    }
return _sem_post(param0);
}

int Originals::pthread_attr_getstacksize(const pthread_attr_t * param0, size_t * param1) {
    if (_pthread_attr_getstacksize== NULL) {
        printf("Thrille: ERROR: original pthread_attr_getstacksize is NULL\n");
        return 0;
    }
return _pthread_attr_getstacksize(param0, param1);
}

int Originals::pthread_create(pthread_t * param0, const pthread_attr_t * param1, void *(* param2)(void *), void * param3) {
    if (_pthread_create== NULL) {
        printf("Thrille: ERROR: original pthread_create is NULL\n");
        return 0;
    }
return _pthread_create(param0, param1, param2, param3);
}

int Originals::sem_timedwait(sem_t * param0, const struct timespec * param1) {
    if (_sem_timedwait== NULL) {
        printf("Thrille: ERROR: original sem_timedwait is NULL\n");
        return 0;
    }
return _sem_timedwait(param0, param1);
}

int Originals::pthread_equal(pthread_t param0, pthread_t param1) {
    if (_pthread_equal== NULL) {
        printf("Thrille: ERROR: original pthread_equal is NULL\n");
        return 0;
    }
return _pthread_equal(param0, param1);
}

int Originals::pthread_spin_init(pthread_spinlock_t * param0, int param1) {
    if (_pthread_spin_init== NULL) {
        printf("Thrille: ERROR: original pthread_spin_init is NULL\n");
        return 0;
    }
return _pthread_spin_init(param0, param1);
}

int Originals::sched_yield() {
    if (_sched_yield== NULL) {
        printf("Thrille: ERROR: original sched_yield is NULL\n");
        return 0;
    }
return _sched_yield();
}

int Originals::pthread_key_create(pthread_key_t * param0, void (* param1)(void *)) {
    if (_pthread_key_create== NULL) {
        printf("Thrille: ERROR: original pthread_key_create is NULL\n");
        return 0;
    }
return _pthread_key_create(param0, param1);
}

int Originals::pthread_mutexattr_destroy(pthread_mutexattr_t * param0) {
    if (_pthread_mutexattr_destroy== NULL) {
        printf("Thrille: ERROR: original pthread_mutexattr_destroy is NULL\n");
        return 0;
    }
return _pthread_mutexattr_destroy(param0);
}

int Originals::pthread_cond_timedwait(pthread_cond_t * param0, pthread_mutex_t * param1, const struct timespec * param2) {
    if (_pthread_cond_timedwait== NULL) {
        printf("Thrille: ERROR: original pthread_cond_timedwait is NULL\n");
        return 0;
    }
return _pthread_cond_timedwait(param0, param1, param2);
}

int Originals::pthread_condattr_getclock(const pthread_condattr_t * param0, clockid_t * param1) {
    if (_pthread_condattr_getclock== NULL) {
        printf("Thrille: ERROR: original pthread_condattr_getclock is NULL\n");
        return 0;
    }
return _pthread_condattr_getclock(param0, param1);
}

int Originals::pthread_setschedparam(pthread_t param0, int param1, const struct sched_param * param2) {
    if (_pthread_setschedparam== NULL) {
        printf("Thrille: ERROR: original pthread_setschedparam is NULL\n");
        return 0;
    }
return _pthread_setschedparam(param0, param1, param2);
}

int Originals::pthread_attr_setinheritsched(pthread_attr_t * param0, int param1) {
    if (_pthread_attr_setinheritsched== NULL) {
        printf("Thrille: ERROR: original pthread_attr_setinheritsched is NULL\n");
        return 0;
    }
return _pthread_attr_setinheritsched(param0, param1);
}

int Originals::pthread_mutex_unlock(pthread_mutex_t * param0) {
    if (_pthread_mutex_unlock== NULL) {
        printf("Thrille: ERROR: original pthread_mutex_unlock is NULL\n");
        return 0;
    }
return _pthread_mutex_unlock(param0);
}

int Originals::pthread_attr_setschedparam(pthread_attr_t * param0, const struct sched_param * param1) {
    if (_pthread_attr_setschedparam== NULL) {
        printf("Thrille: ERROR: original pthread_attr_setschedparam is NULL\n");
        return 0;
    }
return _pthread_attr_setschedparam(param0, param1);
}

int Originals::pthread_rwlockattr_setpshared(pthread_rwlockattr_t * param0, int param1) {
    if (_pthread_rwlockattr_setpshared== NULL) {
        printf("Thrille: ERROR: original pthread_rwlockattr_setpshared is NULL\n");
        return 0;
    }
return _pthread_rwlockattr_setpshared(param0, param1);
}

int Originals::pthread_attr_setstackaddr(pthread_attr_t * param0, void * param1) {
    if (_pthread_attr_setstackaddr== NULL) {
        printf("Thrille: ERROR: original pthread_attr_setstackaddr is NULL\n");
        return 0;
    }
return _pthread_attr_setstackaddr(param0, param1);
}

int Originals::pthread_mutex_destroy(pthread_mutex_t * param0) {
    if (_pthread_mutex_destroy== NULL) {
        printf("Thrille: ERROR: original pthread_mutex_destroy is NULL\n");
        return 0;
    }
return _pthread_mutex_destroy(param0);
}

int Originals::pthread_attr_getschedpolicy(const pthread_attr_t * param0, int * param1) {
    if (_pthread_attr_getschedpolicy== NULL) {
        printf("Thrille: ERROR: original pthread_attr_getschedpolicy is NULL\n");
        return 0;
    }
return _pthread_attr_getschedpolicy(param0, param1);
}

int Originals::pthread_setcanceltype(int param0, int * param1) {
    if (_pthread_setcanceltype== NULL) {
        printf("Thrille: ERROR: original pthread_setcanceltype is NULL\n");
        return 0;
    }
return _pthread_setcanceltype(param0, param1);
}

int Originals::sigwait(const sigset_t * param0, int * param1) {
    if (_sigwait== NULL) {
        printf("Thrille: ERROR: original sigwait is NULL\n");
        return 0;
    }
return _sigwait(param0, param1);
}

int Originals::pthread_rwlock_init(pthread_rwlock_t * param0, const pthread_rwlockattr_t * param1) {
    if (_pthread_rwlock_init== NULL) {
        printf("Thrille: ERROR: original pthread_rwlock_init is NULL\n");
        return 0;
    }
return _pthread_rwlock_init(param0, param1);
}

int Originals::pthread_rwlock_unlock(pthread_rwlock_t * param0) {
    if (_pthread_rwlock_unlock== NULL) {
        printf("Thrille: ERROR: original pthread_rwlock_unlock is NULL\n");
        return 0;
    }
return _pthread_rwlock_unlock(param0);
}

int Originals::pthread_mutexattr_setprotocol(pthread_mutexattr_t * param0, int param1) {
    if (_pthread_mutexattr_setprotocol== NULL) {
        printf("Thrille: ERROR: original pthread_mutexattr_setprotocol is NULL\n");
        return 0;
    }
return _pthread_mutexattr_setprotocol(param0, param1);
}

int Originals::pthread_mutex_getprioceiling(const pthread_mutex_t * param0, int * param1) {
    if (_pthread_mutex_getprioceiling== NULL) {
        printf("Thrille: ERROR: original pthread_mutex_getprioceiling is NULL\n");
        return 0;
    }
return _pthread_mutex_getprioceiling(param0, param1);
}

int Originals::sem_trywait(sem_t * param0) {
    if (_sem_trywait== NULL) {
        printf("Thrille: ERROR: original sem_trywait is NULL\n");
        return 0;
    }
return _sem_trywait(param0);
}

int Originals::pthread_attr_getstackaddr(const pthread_attr_t * param0, void ** param1) {
    if (_pthread_attr_getstackaddr== NULL) {
        printf("Thrille: ERROR: original pthread_attr_getstackaddr is NULL\n");
        return 0;
    }
return _pthread_attr_getstackaddr(param0, param1);
}

int Originals::pthread_mutexattr_settype(pthread_mutexattr_t * param0, int param1) {
    if (_pthread_mutexattr_settype== NULL) {
        printf("Thrille: ERROR: original pthread_mutexattr_settype is NULL\n");
        return 0;
    }
return _pthread_mutexattr_settype(param0, param1);
}

int Originals::pthread_getcpuclockid(pthread_t param0, clockid_t * param1) {
    if (_pthread_getcpuclockid== NULL) {
        printf("Thrille: ERROR: original pthread_getcpuclockid is NULL\n");
        return 0;
    }
return _pthread_getcpuclockid(param0, param1);
}

int Originals::pthread_cond_destroy(pthread_cond_t * param0) {
    if (_pthread_cond_destroy== NULL) {
        printf("Thrille: ERROR: original pthread_cond_destroy is NULL\n");
        return 0;
    }
return _pthread_cond_destroy(param0);
}

int Originals::pthread_spin_lock(pthread_spinlock_t * param0) {
    if (_pthread_spin_lock== NULL) {
        printf("Thrille: ERROR: original pthread_spin_lock is NULL\n");
        return 0;
    }
return _pthread_spin_lock(param0);
}

int Originals::pthread_once(pthread_once_t * param0, void (* param1)(void)) {
    if (_pthread_once== NULL) {
        printf("Thrille: ERROR: original pthread_once is NULL\n");
        return 0;
    }
return _pthread_once(param0, param1);
}

int Originals::pthread_atfork(void (* param0)(void), void (* param1)(void), void(* param2)(void)) {
    if (_pthread_atfork== NULL) {
        printf("Thrille: ERROR: original pthread_atfork is NULL\n");
        return 0;
    }
return _pthread_atfork(param0, param1, param2);
}

int Originals::pthread_attr_setscope(pthread_attr_t * param0, int param1) {
    if (_pthread_attr_setscope== NULL) {
        printf("Thrille: ERROR: original pthread_attr_setscope is NULL\n");
        return 0;
    }
return _pthread_attr_setscope(param0, param1);
}

void * Originals::pthread_getspecific(pthread_key_t param0) {
    if (_pthread_getspecific== NULL) {
        printf("Thrille: ERROR: original pthread_getspecific is NULL\n");
        return NULL;
    }
return _pthread_getspecific(param0);
}

int Originals::pthread_attr_destroy(pthread_attr_t * param0) {
    if (_pthread_attr_destroy== NULL) {
        printf("Thrille: ERROR: original pthread_attr_destroy is NULL\n");
        return 0;
    }
return _pthread_attr_destroy(param0);
}

int Originals::pthread_mutex_timedlock(pthread_mutex_t * param0, const struct timespec * param1) {
    if (_pthread_mutex_timedlock== NULL) {
        printf("Thrille: ERROR: original pthread_mutex_timedlock is NULL\n");
        return 0;
    }
return _pthread_mutex_timedlock(param0, param1);
}

int Originals::pthread_barrier_init(pthread_barrier_t * param0, const pthread_barrierattr_t * param1, unsigned param2) {
    if (_pthread_barrier_init== NULL) {
        printf("Thrille: ERROR: original pthread_barrier_init is NULL\n");
        return 0;
    }
return _pthread_barrier_init(param0, param1, param2);
}

int Originals::pthread_setschedprio(pthread_t param0, int param1) {
    if (_pthread_setschedprio== NULL) {
        printf("Thrille: ERROR: original pthread_setschedprio is NULL\n");
        return 0;
    }
return _pthread_setschedprio(param0, param1);
}

int Originals::pthread_condattr_setclock(pthread_condattr_t * param0, clockid_t param1) {
    if (_pthread_condattr_setclock== NULL) {
        printf("Thrille: ERROR: original pthread_condattr_setclock is NULL\n");
        return 0;
    }
return _pthread_condattr_setclock(param0, param1);
}

int Originals::pthread_cond_broadcast(pthread_cond_t * param0) {
    if (_pthread_cond_broadcast== NULL) {
        printf("Thrille: ERROR: original pthread_cond_broadcast is NULL\n");
        return 0;
    }
return _pthread_cond_broadcast(param0);
}

int Originals::pthread_spin_unlock(pthread_spinlock_t * param0) {
    if (_pthread_spin_unlock== NULL) {
        printf("Thrille: ERROR: original pthread_spin_unlock is NULL\n");
        return 0;
    }
return _pthread_spin_unlock(param0);
}

int Originals::pthread_rwlock_rdlock(pthread_rwlock_t * param0) {
    if (_pthread_rwlock_rdlock== NULL) {
        printf("Thrille: ERROR: original pthread_rwlock_rdlock is NULL\n");
        return 0;
    }
return _pthread_rwlock_rdlock(param0);
}

int Originals::pthread_rwlock_wrlock(pthread_rwlock_t * param0) {
    if (_pthread_rwlock_wrlock== NULL) {
        printf("Thrille: ERROR: original pthread_rwlock_wrlock is NULL\n");
        return 0;
    }
return _pthread_rwlock_wrlock(param0);
}

int Originals::pthread_join(pthread_t param0, void ** param1) {
    if (_pthread_join== NULL) {
        printf("Thrille: ERROR: original pthread_join is NULL\n");
        return 0;
    }
return _pthread_join(param0, param1);
}

int Originals::pthread_cond_init(pthread_cond_t * param0, const pthread_condattr_t * param1) {
    if (_pthread_cond_init== NULL) {
        printf("Thrille: ERROR: original pthread_cond_init is NULL\n");
        return 0;
    }
return _pthread_cond_init(param0, param1);
}

int Originals::pthread_attr_getguardsize(const pthread_attr_t * param0, size_t * param1) {
    if (_pthread_attr_getguardsize== NULL) {
        printf("Thrille: ERROR: original pthread_attr_getguardsize is NULL\n");
        return 0;
    }
return _pthread_attr_getguardsize(param0, param1);
}

int Originals::pthread_barrierattr_init(pthread_barrierattr_t * param0) {
    if (_pthread_barrierattr_init== NULL) {
        printf("Thrille: ERROR: original pthread_barrierattr_init is NULL\n");
        return 0;
    }
return _pthread_barrierattr_init(param0);
}

int Originals::pthread_rwlock_tryrdlock(pthread_rwlock_t * param0) {
    if (_pthread_rwlock_tryrdlock== NULL) {
        printf("Thrille: ERROR: original pthread_rwlock_tryrdlock is NULL\n");
        return 0;
    }
return _pthread_rwlock_tryrdlock(param0);
}

int Originals::pthread_attr_setschedpolicy(pthread_attr_t * param0, int param1) {
    if (_pthread_attr_setschedpolicy== NULL) {
        printf("Thrille: ERROR: original pthread_attr_setschedpolicy is NULL\n");
        return 0;
    }
return _pthread_attr_setschedpolicy(param0, param1);
}

int Originals::pthread_barrierattr_destroy(pthread_barrierattr_t * param0) {
    if (_pthread_barrierattr_destroy== NULL) {
        printf("Thrille: ERROR: original pthread_barrierattr_destroy is NULL\n");
        return 0;
    }
return _pthread_barrierattr_destroy(param0);
}

pthread_t Originals::pthread_self() {
    if (_pthread_self== NULL) {
        printf("Thrille: ERROR: original pthread_self is NULL\n");
        pthread_t * tid = new pthread_t;
        return *tid;
    }
return _pthread_self();
}

int Originals::pthread_attr_getdetachstate(const pthread_attr_t * param0, int * param1) {
    if (_pthread_attr_getdetachstate== NULL) {
        printf("Thrille: ERROR: original pthread_attr_getdetachstate is NULL\n");
        return 0;
    }
return _pthread_attr_getdetachstate(param0, param1);
}

int Originals::pthread_rwlock_timedwrlock(pthread_rwlock_t * param0, const struct timespec * param1) {
    if (_pthread_rwlock_timedwrlock== NULL) {
        printf("Thrille: ERROR: original pthread_rwlock_timedwrlock is NULL\n");
        return 0;
    }
return _pthread_rwlock_timedwrlock(param0, param1);
}

int Originals::pthread_mutexattr_setprioceiling(pthread_mutexattr_t * param0, int param1) {
    if (_pthread_mutexattr_setprioceiling== NULL) {
        printf("Thrille: ERROR: original pthread_mutexattr_setprioceiling is NULL\n");
        return 0;
    }
return _pthread_mutexattr_setprioceiling(param0, param1);
}



