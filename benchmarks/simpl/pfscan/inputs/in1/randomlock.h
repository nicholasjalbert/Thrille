/*
 * randomlock - implementation for lock races
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */

#ifndef _RANDOMLOCK_H_
#define _RANDOMLOCK_H_


#include "randomactive.h"

class RandomLockTester: public RandomActiveTester{
    private:
        void * target1;
        void * target2;

        virtual void setTestingTargets(); 
        
    public:

        RandomLockTester(thrID myself); 

        virtual ~RandomLockTester();

    protected:
        virtual void handleBeforeMutexLock(thrID, pthread_mutex_t *,
                void *, bool);
        virtual void handleAfterMutexUnlock(thrID, pthread_mutex_t *,
                bool);
        virtual bool reenableThreadIfLegal(thrID);

        virtual bool lockIsInvalid(pthread_mutex_t *);
        friend class RandomLockTesterTestSuite;
};

#endif
