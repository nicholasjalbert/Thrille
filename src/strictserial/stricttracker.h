/**
 * stricttracker - tracks the status of a program's execution, implements a 
 * strict scheduler (fails when replay log is finished)
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _STRICTTRACKER_H_
#define _STRICTTRACKER_H_

#include "../serializer/executiontracker.h"


class StrictTracker: public ExecutionTracker {
    public:
        StrictTracker(thrID myself);
        virtual ~StrictTracker();
        bool startSynchro;
        int successful_comparisons;
        int skipped_comparisons;

    protected:
        virtual thrID pickNextSchedulingChoice(SchedPointInfo *);
        virtual void compareScheduleSynchronization(string, string); 
        friend class StrictTrackerTestSuite;


};


#endif

