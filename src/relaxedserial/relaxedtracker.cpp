#define UNLOCKASSERT
#include "relaxedtracker.h"

RelaxedTracker::RelaxedTracker(thrID myself) : ExecutionTracker(myself) {
    delete log;
    log = new RelaxedLogger("my-schedule", "thrille-relaxed-sched");
    currentThreadToSchedule = UNKNOWN_THREAD;
}

RelaxedTracker::~RelaxedTracker() {
}

bool RelaxedTracker::checkAddressOfContextSwitch(thrID myself,
        void * ret_addr) {

    if (ret_addr == addrPreempt) {
        passPreempt--;
    }
    if (passPreempt < 0) {
        if (log->getPrint()) {
            printf("Context Switch because Thread %d ", myself);
            printf("hit ctxtpoint\n");
            printf("Thread %d was scheduled ", myself);
            printf("%d times\n", timesScheduled);
        }
        return true;
    }
    return false;
}

bool RelaxedTracker::checkTimesScheduled(thrID myself,
        void * ret_addr) {
    if (timesToSchedule == timesScheduled) {
        if (log->getPrint()) {
            printf("Thread %d ", myself);
            printf("has run out of times to be scheduled\n");
            printf("Thread %d was scheduled ", myself);
            printf("%d times\n", timesScheduled);
        }
        return true;
    }
    return false;
}

void RelaxedTracker::scheduleAdherenceCheck() {
    safe_assert(timesToSchedule > timesScheduled);
}

void RelaxedTracker::noScheduleToFollow(SchedPointInfo * s) {
    bool canContinueSchedule = false;
    safe_assert(canContinueSchedule);
}

bool RelaxedTracker::mustRegenerateSchedule(thrID myself, void * ret_addr) {
    bool must_regenerate = checkAddressOfContextSwitch(myself, ret_addr);
    must_regenerate = must_regenerate || checkTimesScheduled(myself, ret_addr);
    must_regenerate = must_regenerate || isDisabled(currentThreadToSchedule);
    return must_regenerate;
}


thrID RelaxedTracker::schedule(SchedPointInfo * s) {
    void * ret_addr = s->ret_addr;
    string type = s->type;
    thrID myself = s->thread;
    map<thrID, bool> en_map = s->enabled;

    if (currentThreadToSchedule == UNKNOWN_THREAD) {
        regenerateRelaxedSchedule();
    }

    bool regenerate = mustRegenerateSchedule(myself, ret_addr);

    if (regenerate) {
        // iterate over schedule until an enabled thread is found
        do {
            deadlockCheck();
            regenerateRelaxedSchedule();
            if (currentThreadToSchedule == INVALID_THREAD) {
                noScheduleToFollow(s);
            }
        } while (isDisabled(currentThreadToSchedule));
    }

    deadlockCheck();
    scheduleAdherenceCheck();
    recordScheduleInfo(myself, currentThreadToSchedule, type, ret_addr);

    s->enabled = enable_map;
    launchThread(currentThreadToSchedule, s);

    timesScheduled++;
    return currentThreadToSchedule; 
}


void RelaxedTracker::regenerateRelaxedSchedule() {
    RelaxedLogger * mylogger = dynamic_cast<RelaxedLogger *>(log);
    RelaxedSchedStruct r = mylogger->regenerateSched();
    currentThreadToSchedule = r.current;
    timesToSchedule = r.max;
    addrPreempt = r.addr;
    passPreempt = r.pass;
    timesScheduled = 0;
}

