/**
 * fairscheduler - another attempt at a compact/reusable fair scheduler module
 *
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu)
 *
 * <Legal matter>
 */

#ifndef _FAIRSCHEDULER_H_
#define _FAIRSCHEUDLER_H_

#include <set>
#include <map>
#include "../serializer/serializertypes.h"

class FairScheduler {
    public:
        FairScheduler();
        virtual ~FairScheduler();
        virtual thrID nonpreemptiveFairSchedule(SchedPointInfo *);
    protected:
        map<thrID, set<thrID> > fair_priority;
        map<thrID, set<thrID> > fair_enabled;
        map<thrID, set<thrID> > fair_disabled;
        map<thrID, set<thrID> > fair_scheduled;
        set<thrID> prev_ES;

        virtual map<thrID, bool> getFairEnabledMap(SchedPointInfo *);
        virtual void updateDataStructures(SchedPointInfo *);
        virtual bool isFairEnabled(thrID, map<thrID, bool>);

        
        friend class AddrTracker;
};

#endif

