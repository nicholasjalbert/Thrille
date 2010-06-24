/**
 * dfailtracker - tracks the status of a program's execution, implements a 
 * relaxed schedule that is specificed simply by 
 * [statement iid to switch at] : [number of times to pass iid before switch]
 *
 *
 * NOTE: when a thread becomes disabled before the end of its TEI
 * we fail -- for use with forward consolidation 2.0
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _DFAILTRACKER_H_
#define _DFAILTRACKER_H_

#include "../relaxedserial/relaxedlogger.h"
#include "../addrserial/addrtracker.h"


class DfailTracker: public AddrTracker {
    public:
        DfailTracker(thrID myself);
        virtual ~DfailTracker();

    protected:

        virtual bool mustRegenerateSchedule(thrID, void *); 
        
        friend class DfailTrackerTestSuite;

};


#endif

