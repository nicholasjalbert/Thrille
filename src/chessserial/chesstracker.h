/**
 * chesstracker - tracks the status of a program's execution, implements 
 * non-preemptive fair scheduling 
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _CHESSTRACKER_H_
#define _CHESSTRACKER_H_

#include "../serializer/executiontracker.h"
#include "../addrserial/fairscheduler.h"


class ChessTracker: public ExecutionTracker {
    public:
        ChessTracker(thrID myself);
        virtual ~ChessTracker();

    protected:
        FairScheduler * fairscheduler;
        
        virtual void followingChoiceFromLog(SchedPointInfo *); 
        virtual thrID pickNextSchedulingChoice(SchedPointInfo *);
        virtual void launchThread(thrID, SchedPointInfo *);

        friend class ChessTrackerTestSuite;
};


#endif

