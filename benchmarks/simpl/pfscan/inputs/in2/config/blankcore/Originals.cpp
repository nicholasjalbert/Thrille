#include "Originals.h"
#include "libpth.h"
#include <dlfcn.h>

#define init_original(f, type) { \
    _##f = (type) dlsym(RTLD_NEXT, #f); \
    if (_##f == NULL) throw 48; \
}

volatile bool Originals::_initialized = false;

pthread_once_t Originals::_once_control = PTHREAD_ONCE_INIT;
/*
int (* volatile Originals::_usleep) (useconds_t) = NULL;
unsigned int (* volatile Originals::_sleep) (unsigned int) = NULL;
int (* volatile Originals::_sem_init) (sem_t *, int, unsigned int) = NULL;
int (* volatile Originals::_sem_destroy) (sem_t *) = NULL;
int (* volatile Originals::_sem_getvalue) (sem_t *, int *) = NULL;
int (* volatile Originals::_sem_post) (sem_t *) = NULL;
int (* volatile Originals::_sem_wait) (sem_t *) = NULL;
int (* volatile Originals::_sem_trywait) (sem_t *) = NULL;
int (* volatile Originals::_sem_timedwait) (sem_t *, const struct timespec *) = NULL;
int (* volatile Originals::_sched_yield) (void) = NULL;
*/


PYTHON_INIT_NULL

volatile bool Originals::is_initialized()
{
    return _initialized;
}

void Originals::_initialize()
{
    safe_assert(!_initialized);

/*
    init_original(usleep, int (* volatile) (useconds_t));
    init_original(sleep, unsigned int (* volatile) (unsigned int));
    init_original(sem_init, int (* volatile) (sem_t *, int, unsigned int));
    init_original(sem_destroy, int (* volatile) (sem_t *));
    init_original(sem_getvalue, int (* volatile) (sem_t *, int *));
    init_original(sem_post, int (* volatile) (sem_t *));
    init_original(sem_wait, int (* volatile) (sem_t *));
    init_original(sem_trywait, int (* volatile) (sem_t *));
    init_original(sem_timedwait, 
            int (* volatile) (sem_t *, const struct timespec *));
    init_original(sched_yield, int (* volatile) (void));
    */

    PYTHON_INIT_ORIGINALS

        _initialized = true;
}


void Originals::initialize()
{
    if (_initialized) return;

    int (*local_pthread_once) (pthread_once_t *, void (*)(void)) = NULL;

    local_pthread_once = (int (*) (pthread_once_t *, void (*)(void))) dlsym(RTLD_NEXT, "pthread_once");

    safe_assert(local_pthread_once != NULL);

    const int rc = local_pthread_once(&_once_control, _initialize);

    safe_assert (rc == 0);
    safe_assert (_initialized);

}

/*
int Originals::usleep(useconds_t param0) {
    safe_assert(_usleep != NULL);
    return _usleep(param0);
}

unsigned int Originals::sleep(unsigned int param0) {
    safe_assert(_sleep != NULL);
    return _sleep(param0);
}

int Originals::sem_init(sem_t * param0, int param1, unsigned int param2) {
    safe_assert(_sem_init != NULL);
    return _sem_init(param0, param1, param2);
}

int Originals::sem_destroy(sem_t * param0) {
    safe_assert(_sem_destroy != NULL);
    return _sem_destroy(param0);
}

int Originals::sem_getvalue(sem_t * param0, int * param1) {
    safe_assert(_sem_getvalue != NULL);
    return _sem_getvalue(param0, param1);
}

int Originals::sem_post(sem_t * param0) {
    safe_assert(_sem_post != NULL);
    return _sem_post(param0);
}

int Originals::sem_wait(sem_t * param0) {
    safe_assert(_sem_wait != NULL);
    return _sem_wait(param0);
}

int Originals::sem_trywait(sem_t * param0) {
    safe_assert(_sem_trywait != NULL);
    return _sem_trywait(param0);
}

int Originals::sem_timedwait(sem_t * param0, const struct timespec * param1) {
    safe_assert(_sem_timedwait != NULL);
    return _sem_timedwait(param0, param1);
}

int Originals::sched_yield(void) {
    safe_assert(_sched_yield != NULL);
    return sched_yield();
}
*/

PYTHON_ORIG_IMPL

