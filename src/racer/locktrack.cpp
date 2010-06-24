#include "locktrack.h"

lockSet LockSetTracker::getLockSet(thrID thread) {
    lockSet thels = lockmap[thread];
    return thels;
}

void LockSetTracker::lockBefore(thrID thread, pthread_mutex_t * lock) {
    lockSet thels = lockmap[thread];
    thels.insert(lock);
    lockmap[thread] = thels;
}


void LockSetTracker::unlockAfter(thrID thread, pthread_mutex_t * lock) {
    lockSet thels = lockmap[thread];
    lockSet::iterator itr = thels.find(lock);
    if (itr == thels.end()) {
        printf("Lock set error (missing lock)\n");
    } else {
        thels.erase(itr);
        lockmap[thread] = thels;
    }
}

bool LockSetTracker::isIntersectionEmpty(lockSet l1, lockSet l2) {
    lockSet::iterator itr;
    for (itr = l1.begin(); itr != l1.end(); itr++) {
        if (l2.find(*itr) != l2.end()) {
            return false;
        } 
    }
    return true;
}

