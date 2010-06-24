/**
 * locktrack.h - Lockset Tracking 
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */

#ifndef _LOCKTRACK_H_
#define _LOCKTRACK_H_

#include "hybridtypes.h"

class LockSetTracker {
    private:
       lsContainer lockmap; 
        
    public:
        LockSetTracker() {
        }

        ~LockSetTracker() {
        }

        lockSet getLockSet(thrID thread);
        void lockBefore(thrID thread, pthread_mutex_t * lock);
        void unlockAfter (thrID thread, pthread_mutex_t * lock);
        bool isIntersectionEmpty(lockSet l1, lockSet l2);
        


};

#endif
