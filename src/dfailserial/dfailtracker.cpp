#define UNLOCKASSERT
#include "dfailtracker.h"

DfailTracker::DfailTracker(thrID myself) : AddrTracker(myself) {
}

DfailTracker::~DfailTracker() {
}



bool DfailTracker::mustRegenerateSchedule(thrID myself, void * ret_addr) {
    bool must_regenerate = checkAddressOfContextSwitch(myself, ret_addr);
    must_regenerate = must_regenerate || checkTimesScheduled(myself, ret_addr);
    if (! must_regenerate) {
        deadlockCheck();
        bool threadNotDisabledInTEI = isEnabled(currentThreadToSchedule);
        safe_assert(threadNotDisabledInTEI);
    }
    return must_regenerate;
}
