#define UNLOCKASSERT
#include "stricttracker.h"
        
StrictTracker::StrictTracker(thrID myself) : ExecutionTracker(myself) {
    startSynchro = false;
    successful_comparisons = 0;
    skipped_comparisons = 0;
}

StrictTracker::~StrictTracker() {
    printf("[STRICT] skipped %d comparisons ", skipped_comparisons);
    printf("to establish synchronization\n");
    printf("[STRICT] scheduling points were successfully synchronized ");
    printf("%d times\n", successful_comparisons);

}

thrID StrictTracker::pickNextSchedulingChoice(SchedPointInfo * s) {
    deadlockCheck();
    bool didNotGoPastEndOfReplayLog = false;
    safe_assert(didNotGoPastEndOfReplayLog);
    return INVALID_THREAD;
}


void StrictTracker::compareScheduleSynchronization(string now, string then) {
    if (! startSynchro && now != then) {
        skipped_comparisons++;
        return;
    }
    startSynchro = true;
    if (now != then) {
        bool schedulesAreSynchronized = false;
        printf("now %s then %s\n", now.c_str(), then.c_str());
        safe_assert(schedulesAreSynchronized);
    } else {
        successful_comparisons++;
    }
}
