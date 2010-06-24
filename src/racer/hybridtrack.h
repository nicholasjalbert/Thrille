/**
 * hybridtrack.h -- Hybrid race detection
 *  
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */

#ifndef _HYTRACK_H_
#define _HYTRACK_H_

#include "hybridtypes.h"
#include "vctrack.h"
#include "locktrack.h"

class HybridRaceTracker {
    protected:
        VectorClockTracker vsTracker;
        LockSetTracker lsTracker;
        map<void *, vector<dataRaceEvent> > eventset;
        vector<dataRaceRecord> foundraces;
        unsigned int wr_profile_size; 
        unsigned int rd_profile_size;
        
        virtual bool isRacing(raceEvent e1, raceEvent e2);

    public:
        HybridRaceTracker(); 
        HybridRaceTracker(unsigned int, unsigned int); 
        virtual ~HybridRaceTracker(); 

        virtual unsigned int getWrProfileSize();
        virtual unsigned int getRdProfileSize();
        virtual void afterJoin(thrID me, thrID thrjoined);
        virtual void beforeSignal(thrID me, pthread_cond_t * cond);
        virtual void afterWait(thrID me, pthread_cond_t * cond);
        virtual void beforeCreate(thrID me, thrID child); 
        virtual void lockBefore(thrID thread, pthread_mutex_t *);
        virtual void unlockAfter (thrID thread, pthread_mutex_t *);
        virtual lockSet getLockSet(thrID thread);
        virtual vectorClock getVectorClock(thrID thread);
        virtual void addLockEvent(lockRaceEvent);
        virtual void addEvent(dataRaceEvent);

        virtual map<void *, vector<dataRaceEvent> > getRaceEvents();
        virtual void checkRace(dataRaceEvent);
        virtual vector<dataRaceRecord> getRaces();
        virtual void dumpRaces(ofstream *);
        virtual void outputRaces(); 


};

#endif
