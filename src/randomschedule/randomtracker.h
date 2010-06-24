/**
 * randomtracker - tracks the status of a program's execution, implements a 
 * random scheduler
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _RANDTRACKER_H_
#define _RANDTRACKER_H_

#include "../serializer/executiontracker.h"


class RandomTracker: public ExecutionTracker {
    public:
        RandomTracker(thrID myself);
        virtual ~RandomTracker();
        int chanceOfPreempt;

    protected:
        virtual thrID pickNextSchedulingChoice(SchedPointInfo *);

        virtual thrID selectThreadWaitingOnCond(SignalPointInfo *);
        virtual thrID chooseRandomThread(map<thrID, bool>::iterator,
                map<thrID, bool>::iterator);

        friend class RandomTrackerTestSuite;


};


#endif

