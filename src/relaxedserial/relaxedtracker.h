/**
 * relaxedtracker - tracks the status of a program's execution, implements a 
 * relaxed feasability function
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _RELAXEDTRACKER_H_
#define _RELAXEDTRACKER_H_

#include "../serializer/executiontracker.h"
#include "relaxedlogger.h"


class RelaxedTracker: public ExecutionTracker {
    public:
        RelaxedTracker(thrID myself);
        virtual ~RelaxedTracker();

    protected:
        int timesScheduled;
        thrID currentThreadToSchedule;
        int timesToSchedule;
        void * addrPreempt;
        int passPreempt;
        vector<thrID> threadsToSignal;

        virtual thrID schedule(SchedPointInfo * s);
        virtual bool checkAddressOfContextSwitch(thrID, void *);
        virtual bool checkTimesScheduled(thrID, void *);
        virtual void noScheduleToFollow(SchedPointInfo *);
        virtual bool mustRegenerateSchedule(thrID, void *);

        virtual void scheduleAdherenceCheck();
        virtual void regenerateRelaxedSchedule();
        friend class RelaxedTrackerTestSuite;


};


#endif

