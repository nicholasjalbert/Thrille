#define UNLOCKASSERT
#include "testertracker.h"

TesterTracker::TesterTracker(thrID myself) : RelaxedTracker(myself) {
    delete log;
    log = new RelaxedLogger("my-schedule", "thrille-relaxed-sched");
}

TesterTracker::~TesterTracker() {
}

bool TesterTracker::mustRegenerateSchedule(thrID myself, void * ret_addr) {
    bool must_regenerate = checkAddressOfContextSwitch(myself, ret_addr);
    must_regenerate = must_regenerate || checkTimesScheduled(myself, ret_addr);
    if (! must_regenerate) {
        safe_assert(isEnabled(currentThreadToSchedule));
    }
    return must_regenerate;
}

void TesterTracker::regenerateRelaxedSchedule() {
    RelaxedLogger * mylogger = dynamic_cast<RelaxedLogger *>(log);
    RelaxedSchedStruct r = mylogger->regenerateSched();
    currentThreadToSchedule = r.current;
    timesToSchedule = r.max;
    addrPreempt = r.addr;
    passPreempt = r.pass;
    timesScheduled = 0;
    safe_assert(isEnabled(currentThreadToSchedule));
}


