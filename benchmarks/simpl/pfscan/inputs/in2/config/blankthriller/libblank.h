/**
* libblank - [purpose] 
* 
* Author -  [name] [email]
*
* <Legal matter>
*/

#ifndef _LIBBLANK_H_
#define _LIBBLANK_H_

#include "../thrille-core/libpth.h"
#include "tlsblank.h"

class BlankHandler : public Handler {
    private: 

    public:
        BlankHandler() : Handler() {
            dbgPrint("Starting Blank Thriller...\n");
       }

        virtual ~BlankHandler() {
            dbgPrint("Ending Blank Thriller...\n");
       }



    protected:
        virtual void AfterInitialize();
        virtual void ThreadStart(void * (*) (void *), void * arg); 
        virtual void ThreadFinish(void * (*) (void *), void * status); 

        virtual bool BeforeCreate(void * ret_addr,
                pthread_t *, 
                const pthread_attr_t *,
                void *(*)(void *),
                void *,
                ThreadInfo * &); 
        virtual int SimulateCreate(void * ret_addr,
                pthread_t *,
                const pthread_attr_t *,
                void *(*)(void *),
                void *,
                ThreadInfo * &);
        virtual void AfterCreate(void * ret_addr,
                int,
                pthread_t *,
                const pthread_attr_t *,
                void *(*)(void *),
                void * );


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
         *               function's args, with an added void * param 
         *               in the first position (function's return addr).
         *
         *     Simulate*: returns return type of original function and 
         *                takes the original function's args with an added
         *                void * param in the first position (function's 
         *                return addr).
         *
         *     After*: returns void, takes as arguments the void * return addr
         *             of the original function in the first position, 
         *             the returntype of the 
         *             original pthread function (2nd argument) and then the 
         *             original function's args 
         *    
         *     e.g. int pthread_mutex_lock(pthread_mutex_t * mutex) has 
         *          the handles:
         *
         *     virtual bool BeforeMutexLock(void *, pthread_mutex_t *);
         *     virtual int SimulateMutexLock(void *, pthread_mutex_t *);
         *     virtual void AfterMutexLock(void *, int, pthread_mutex_t *);
         **/
        
        // Example of interposition for arbitrary functions 
        // See interpose.cpp for more
        //
        // friend void myMemoryRead(int, void*);
        // friend void myMemoryWrite(int, void*);
        friend class LibBlankTestSuite;
};

#endif
