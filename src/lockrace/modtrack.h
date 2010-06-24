/**
 * modtrack.h -- modified hybrid race detection that tracks lock contention
 *  
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */

#ifndef _MODTRACK_H_
#define _MODTRACK_H_

#include "../racer/hybridtrack.h"

class ModifiedRaceTracker : public HybridRaceTracker {
    public:
        ModifiedRaceTracker();
        ModifiedRaceTracker(unsigned int, unsigned int);
        virtual ~ModifiedRaceTracker(); 

    protected:

        void initializationHelper();
        map<void *, int> lock_profile_map;
        int lockprofiling;
        int eventcount;
        map<void *, vector<lockRaceEvent> > lockeventset;
        vector<lockRaceRecord> foundlockraces;
        string outfilename;

        virtual void checkLockRace(lockRaceEvent);
        virtual void beforeSignal(thrID me, pthread_cond_t * cond);
        virtual void afterWait(thrID me, pthread_cond_t * cond);
        virtual void addLockEvent(lockRaceEvent);
        virtual void outputRaces(); 
        virtual void dumpRaces(ofstream *);

};

#endif
