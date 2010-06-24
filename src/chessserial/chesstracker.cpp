#define UNLOCKASSERT
#include "chesstracker.h"
        
ChessTracker::ChessTracker(thrID myself) : ExecutionTracker(myself) {
    fairscheduler = new FairScheduler();
}

ChessTracker::~ChessTracker() {
    delete fairscheduler;
}

void ChessTracker::followingChoiceFromLog(SchedPointInfo * s) {
    fairscheduler->nonpreemptiveFairSchedule(s);
}


thrID ChessTracker::pickNextSchedulingChoice(SchedPointInfo * s) {
    return fairscheduler->nonpreemptiveFairSchedule(s);
}

void ChessTracker::launchThread(thrID target, SchedPointInfo * s) {
    log->fairScheduling(target, s);
    safe_assert(s->enabled[target]);
    safe_assert(s->fenmap[target]); 
    if (cond_map[target] != NULL) {
        was_signalled_map[target] = false;
        waitWakeThread(target);
    } else {
        wakeThread(target);
    }
}


