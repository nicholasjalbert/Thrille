#include "vctrack.h"

void printVectorClock(vectorClock v) {
    vectorClock::iterator itr;
    printf("VC:\n");
    for (itr = v.begin(); itr != v.end(); itr++) {
        printf("\tThr %d is at time %d\n", itr->first, itr->second);
    }
    printf("________________\n");
}

void VectorClockTracker::incrementMyClock(thrID id) {
    vectorClock myclock = vcs[id];
    int currentClock = myclock[id];
    currentClock++;
    myclock[id] = currentClock;
    vcs[id] = myclock;
}

void VectorClockTracker::receiveMessage(thrID thr, messageMap msg) {
    vectorClock newvc = vcs[thr];
    messageMap::iterator itr;
    for (itr = msg.begin(); itr != msg.end(); itr++) {
        vectorClock::iterator vcitr = newvc.find(itr->first);
        if (vcitr != newvc.end()) {
            newvc[itr->first] = vcitr->second > itr->second ? 
                vcitr->second : 
                itr->second;
        } else {
            newvc[itr->first] = itr->second;
        }
    }
    vcs[thr] = newvc;
}

vectorClock VectorClockTracker::getVectorClock(thrID thr) {
    vectorClock newvc = vcs[thr];
    return newvc;
}

bool VectorClockTracker::isHappensBefore(vectorClock before,
                                            vectorClock after) {
    vectorClock::iterator itr;
    for(itr = before.begin(); itr != before.end(); itr++) {
        int comparison = after[itr->first];
        if (itr->second > comparison) {
            return false;
        }
    }
    return true;
}

void VectorClockTracker::afterJoin(thrID me, thrID thrjoined) {
    receiveMessage(me, vcs[thrjoined]);
}

void VectorClockTracker::beforeSignal(thrID me, pthread_cond_t * cond) {
    vectorClock newvc = vcs[me];
    messagemap[cond] = newvc;
    incrementMyClock(me);
}

void VectorClockTracker::afterWait(thrID me, pthread_cond_t * cond) {
    receiveMessage(me, messagemap[cond]);
}

void VectorClockTracker::beforeCreate(thrID me, thrID child) {
    vectorClock mine = vcs[me];
    vcs[child] = mine;
    incrementMyClock(me);
    incrementMyClock(child);
}


