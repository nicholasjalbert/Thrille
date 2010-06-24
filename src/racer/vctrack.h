/**
 * vctrack - Vector Clock Tracking
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */

#ifndef _VCTRACK_H_
#define _VCTRACK_H_

#include "hybridtypes.h"

class VectorClockTracker {
    private:
        vcContainer vcs;
        msgContainer messagemap;

        void incrementMyClock(thrID id);
        void receiveMessage(thrID thr, messageMap msg);
        
    public:
        VectorClockTracker() {
            vectorClock mainthrclock;
            mainthrclock[0] = 1;
            vcs[0] = mainthrclock;
        }

        ~VectorClockTracker() {
        }

        vectorClock getVectorClock(thrID thr);
        bool isHappensBefore(vectorClock before, vectorClock after);
        void afterJoin(thrID me, thrID thrjoined);
        void beforeSignal(thrID me, pthread_cond_t * cond);
        void afterWait(thrID me, pthread_cond_t * cond);
        void beforeCreate(thrID me, thrID child); 
};

void printVectorClock(vectorClock);

#endif
