#include "libpth.h"
#include "Initializer.h"
#include "Originals.h"
#include "ThreadInfo.h"

Handler * volatile pHandler = NULL;

const char* Handler::UNKNOWN = "unknown";
__thread int Handler::insideInst = 0;


void pth_start(void)
{
    if (! Initializer::run()) {
        printf("Initialization failed--are you linking to Pthreads?\n");
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
    ThreadInfo * tInfo;

    args_t(void *(*start_routine_)(void*),
            void * arg_,
            ThreadInfo * tInfo_)
        : start_routine(start_routine_),
        arg(arg_),
        tInfo(tInfo_)
    {}

};

void * my_start_routine(void * my_arg)
{
    args_t * const arg = static_cast<args_t *>(my_arg);

    ThreadInfo::set(arg->tInfo);

    pHandler->ThreadStart(arg->start_routine, arg->arg);

    void * status = arg->start_routine(arg->arg);

    pHandler->ThreadFinish(arg->start_routine, status);

    delete arg;

    return status;
}

//implementation of Thrille private methods

int Handler::thrille_create(void * ret_addr,
        pthread_t* newthread,
        const pthread_attr_t *attr, 
        void *(*start_routine) (void *),
        void *arg) {

    _single_thread = false;
    if(insideInst) { 
        printf("ThrilleError: Inside Instrumentation in pthread_create\n");
        exit(3);
    }
    int ret_val;
    ++insideInst;

    ThreadInfo * tInfo = NULL;

    bool call_original = BeforeCreate(ret_addr,
            newthread,
            attr,
            start_routine,
            arg,
            tInfo);

    args_t * const my_arg = new args_t(start_routine, arg, tInfo);

    if (call_original)

        ret_val = Originals::pthread_create(newthread, 
                attr, 
                my_start_routine, 
                (void *)my_arg);
    else
        ret_val = SimulateCreate(ret_addr,
                newthread, 
                attr, 
                start_routine, 
                arg,
                tInfo); 
    AfterCreate(ret_addr, ret_val, newthread, attr, start_routine, arg);
    --insideInst;
    return ret_val;
}

PYTHON_HANDLER_IMPL

//Implementation of empty Interposition hooks

bool Handler::BeforeCreate(void * ret_addr,
        pthread_t * param0, 
        const pthread_attr_t * param1,
        void *(* param2)(void *),
        void * param3,
        ThreadInfo * & param4) {
    if (safe_mode) {
        bool functionIsImplemented = false;
        safe_assert(functionIsImplemented);
    }
    param4 = new ThreadInfo();
    return true; 
}

int Handler::SimulateCreate(void * ret_addr,
        pthread_t * param0,
        const pthread_attr_t * param1,
        void *(* param2)(void *),
        void * param3,
        ThreadInfo * & param4) {return 0;}


void Handler::AfterCreate(void * ret_addr,
        int ret_val,
        pthread_t * param0,
        const pthread_attr_t * param1,
        void *(* param2)(void *),
        void * param3) { }

PYTHON_EMPTY_HOOKS

//Implmentation of Thrille Interposition methods

int pthread_create (pthread_t* thr,
        const pthread_attr_t *attr, 
        void *(*start_routine) (void *), 
        void *arg) {
    void * ret_addr = __builtin_return_address(0);
    return pHandler->thrille_create(ret_addr, thr, attr, start_routine, arg);
}

PYTHON_INTERPOS_IMPL
