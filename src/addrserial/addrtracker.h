/**
 * addrtracker - tracks the status of a program's execution, implements a 
 * relaxed schedule that is specificed simply by 
 * [statement iid to switch at] : [number of times to pass iid before switch]
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _ADDRTRACKER_H_
#define _ADDRTRACKER_H_

#include "../relaxedserial/relaxedlogger.h"
#include "../relaxedserial/relaxedtracker.h"
#include "fairscheduler.h"
#include <set>


class AddrTracker: public RelaxedTracker {
    public:
        AddrTracker(thrID myself);
        virtual ~AddrTracker();

    protected:
        int timesExpectedToSchedule;
        FairScheduler * fairscheduler;

        
        virtual void scheduleThreadWaitingOnCond(thrID, SignalPointInfo *);
        virtual bool checkTimesScheduled(thrID, void *);
        virtual void noScheduleToFollow(SchedPointInfo *);
        virtual bool checkAddressOfContextSwitch(thrID, void *);
        virtual void scheduleAdherenceCheck();

        friend class AddrTrackerTestSuite;

        // random prototype -------------------------

        thrID chooseRandomThread(map<thrID, bool>::iterator begin,
                map<thrID, bool>::iterator end);

        thrID selectThreadWaitingOnCond(SignalPointInfo * s);


};


#endif

