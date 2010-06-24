/**
 * testertracker - tracks the status of a program's execution, implements a 
 * relaxed schedule that is specificed simply by 
 * [statement iid to switch at] : [number of times to pass iid before switch].
 * fails out if the relaxed schedule specified requires deviation to exhibit 
 * the bug (where deviations are the relaxations implemented by e.g. 
 * addrserial thriller).  This thriller is intended to be used in sanity 
 * checks
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _TESTERTRACKER_H_
#define _TESTERTRACKER_H_

#include "../relaxedserial/relaxedlogger.h"
#include "../relaxedserial/relaxedtracker.h"


class TesterTracker: public RelaxedTracker {
    public:
        TesterTracker(thrID myself);
        virtual ~TesterTracker();

    protected:
        virtual bool mustRegenerateSchedule(thrID, void *);
        virtual void regenerateRelaxedSchedule();
};


#endif

