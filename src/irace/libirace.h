/**
* libirace - [purpose] 
* 
* Author -  [name] [email]
*
* <Legal matter>
*/

#ifndef _LIBIRACE_H_
#define _LIBIRACE_H_

#include "tlsirace.h"
#include "iracetypes.h"
#include "iracer.h"

class IraceHandler : public Handler {
    private: 
        pthread_t * main_thr;
        ThreadTracker * tracker;
        Iracer * myrace;
        pthread_mutex_t * global;

    public:
        IraceHandler() : Handler() {
            dbgPrint("Starting Irace Thriller...\n");
       }

        virtual ~IraceHandler() {
            delete global;
            delete main_thr;
            delete tracker;
            delete myrace;
            dbgPrint("Ending Irace Thriller...\n");
       }



    protected:
        virtual void AfterInitialize();
        virtual void ThreadStart(void * arg); 
        virtual void ThreadFinish(void * status); 
        virtual void glock();
        virtual void gunlock();

        virtual bool BeforeCreate(pthread_t *, 
                const pthread_attr_t *,
                void *(*)(void *),
                void *,
                ThreadInfo * &); 
        virtual thrID getThrID();
        virtual thrID getThrParent();

        virtual void AfterJoin(int, pthread_t, void**);
        virtual bool BeforeMutexUnlock(pthread_mutex_t *);
        virtual void AfterMutexLock(int, pthread_mutex_t *);
        virtual bool BeforeCondSignal(pthread_cond_t *);
        virtual bool BeforeCondBroadcast(pthread_cond_t *);
        virtual void AfterCondWait(int, pthread_cond_t *);
        virtual void AfterCondTimedwait(int, pthread_cond_t *, 
                const struct timespec *);
        virtual void memoryRead(int, void *);
        virtual void memoryWrite(int, void *);
        virtual void initIracer();



        /**
         * Signature of Pthread Handles:
         *     Pattern: remove pthread, remove underscores and capitalize each
         *              letter following underscore. Prepend one of Before, 
         *              After, or Simulate to get the handle name.  Read on for
         *              info about each specific hook and don't forget
         *              it's a virtual function.
         *
         *              e.g. A handle before pthread_mutex_init() becomes 
         *                   BeforeMutexInit().
         *
         *     Before*:  returns a boolean, if boolean is true then the 
         *               original pthread_* function is called, otherwise 
         *               the Simulate* is called.  Takes the original 
         *               function's args.
         *
         *     Simulate*: returns return type of original function and 
         *                takes the original function's args
         *
         *     After*: returns void, takes as arguments the returntype of the 
         *             original pthread function (1st argument) and then the 
         *             original function's args 
         *    
         *     e.g. int pthread_mutex_lock(pthread_mutex_t * mutex) has 
         *          the handles:
         *
         *     virtual bool BeforeMutexLock(pthread_mutex_t *);
         *     virtual int SimulateMutexLock(pthread_mutex_t *);
         *     virtual void AfterMutexLock(int, pthread_mutex_t *);
         **/
        
        // Example of interposition for arbitrary functions 
        // See .cpp for more
        //
        // friend void myMemoryRead(int, void*);
        // friend void myMemoryWrite(int, void*);
        friend class LibIraceTestSuite;
        friend void myMemoryRead(int, void*);
        friend void myMemoryWrite(int, void*);
};

#endif
