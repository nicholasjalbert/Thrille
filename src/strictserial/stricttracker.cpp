#define UNLOCKASSERT
#include "stricttracker.h"
        
StrictTracker::StrictTracker(thrID myself) : ExecutionTracker(myself) {
    startSynchro = false;
}

StrictTracker::~StrictTracker() {
}

thrID StrictTracker::pickNextSchedulingChoice(SchedPointInfo * s) {
    deadlockCheck();
    bool didNotGoPastEndOfReplayLog = false;
    safe_assert(didNotGoPastEndOfReplayLog);
    return INVALID_THREAD;
}


void StrictTracker::compareScheduleSynchronization(string now, string then) {
    if (! startSynchro && now != then) {
        return;
    }
    startSynchro = true;
    if (now != then) {
        bool schedulesAreSynchronized = false;
        printf("now %s then %s\n", now.c_str(), then.c_str());
        safe_assert(schedulesAreSynchronized);
    }
}
