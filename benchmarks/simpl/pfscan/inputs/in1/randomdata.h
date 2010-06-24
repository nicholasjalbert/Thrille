/*
 * randomdata - implementation for data races
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */

#ifndef _RANDOMDATA_H_
#define _RANDOMDATA_H_

#include "randomactive.h"

class RandomDataTester: public RandomActiveTester{
    private:
        void * target1;
        void * target2;
        
        virtual void setTestingTargets();

    public:

        RandomDataTester(thrID myself);

        //for testing purposes only
        RandomDataTester(thrID myself, bool);

        virtual ~RandomDataTester(); 
        
        
    protected:
        virtual void handleMyMemoryRead(thrID, void *, void *);
        virtual void handleMyMemoryWrite(thrID, void *, void *);
        virtual bool reenableThreadIfLegal(thrID);

        friend class RandomDataTesterTestSuite;
};

#endif
