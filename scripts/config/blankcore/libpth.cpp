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

PYTHON_HANDLER_IMPL

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

PYTHON_EMPTY_HOOKS

//Implementation of Thrille Interposition methods

int pthread_create (pthread_t* thr,
        const pthread_attr_t *attr, 
        void *(*start_routine) (void *), 
        void *arg) {
    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_create(ret_addr, thr, attr, start_routine, arg);
}

PYTHON_INTERPOS_IMPL


