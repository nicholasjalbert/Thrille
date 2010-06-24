/**
* libdpor - [purpose] 
* 
* Author -  [name] [email]
*
* <Legal matter>
*/

#ifndef _LIBDPOR_H_
#define _LIBDPOR_H_

#include "../randomschedule/librandomschedule.h"
#include "dportracker.h"
#include "tlsdpor.h"

class DporHandler : public RandomscheduleHandler{
    private: 

    public:
        DporHandler() : RandomscheduleHandler() {
            dbgPrint("Starting Dpor Thriller...\n");
       }

        virtual ~DporHandler() {
            dbgPrint("Ending Dpor Thriller...\n");
       }



    protected:

        virtual ExecutionTracker * getNewExecutionTracker(thrID);
        

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
        friend class LibDporTestSuite;
};

#endif
