
#include "addrtracker.h"

AddrTracker::AddrTracker(thrID myself) : RelaxedTracker(myself) {
    delete log;
    log = new RelaxedLogger("my-schedule", "thrille-relaxed-sched");
    unsigned long schedulehash = log->getLogHash();
    log->seedGenerator(schedulehash);
    fairscheduler = new FairScheduler();
    regenerateRelaxedSchedule();
}

AddrTracker::~AddrTracker() {
    delete fairscheduler;
}



bool AddrTracker::checkTimesScheduled(thrID myself, void * ret_addr) {
    /* if thread is executing for too long--suspect livelock */
    if (timesScheduled > 2 * timesToSchedule) {
        return true;
    }
    return false;
}


void AddrTracker::noScheduleToFollow(SchedPointInfo * s) {
    fairscheduler->updateDataStructures(s);
    map<thrID, bool> fenmap = fairscheduler->getFairEnabledMap(s);
    if (fenmap[s->thread]) {
        currentThreadToSchedule = s->thread;
    } else {
        currentThreadToSchedule = 
            chooseRandomThread(fenmap.begin(), fenmap.end());
    }

    printf("fair schedule schedule %d after %d\n", currentThreadToSchedule,
            s->thread);
    timesToSchedule = 1;
    timesScheduled = 0;
}

bool AddrTracker::checkAddressOfContextSwitch(thrID myself,
        void * ret_addr) {
    if (ret_addr == addrPreempt) {
        passPreempt--;
    }
    if (passPreempt < 0) {
        return true;
    }
    return false;
}


void AddrTracker::scheduleAdherenceCheck() {
    return;
}

void AddrTracker::scheduleThreadWaitingOnCond(thrID target,
        SignalPointInfo * s) {
    glock();
    thrID tosched = target;
    while (s->cond_map[tosched] != s->cond) {
        tosched = log->getNextSignalChoice();
        if (tosched == INVALID_THREAD) {
            tosched = selectThreadWaitingOnCond(s);
            if (tosched == INVALID_THREAD) {
                gunlock();
                return;
            }
        }
    }
    log->signalScheduling(tosched, s);
    wait_ready_map[tosched] = false;
    waitWakeThread(tosched);
    gunlock();
    waitThreadReady(s->thread, tosched);
}


// functions for rapid prototype of random-ness----------------


thrID AddrTracker::chooseRandomThread(map<thrID, bool>::iterator begin,
        map<thrID, bool>::iterator end) {

    map<thrID, bool>::iterator itr;

    int numberOfChoices= 0;
    for (itr = begin; itr != end; itr++) {
        if (itr->second) {
            numberOfChoices++;
        }
    }

    if (numberOfChoices == 0) {
        return -1;
    }

    int chosenThread = 0;

    if (numberOfChoices > 1) {
        chosenThread = log->getRandomNumber() % numberOfChoices;
    }

    for (itr = begin; itr != end; itr++) {
        if (itr->second) {
            chosenThread--;
            if (chosenThread < 0) {
                return itr->first;
            }
        }
    }
    printf("NOC: %d, chosenThread: %d\n", numberOfChoices, chosenThread);
    safe_assert(false);
    return -1;
}


thrID AddrTracker::selectThreadWaitingOnCond(SignalPointInfo * s) {
    printf("SELECTING RANDOM THREAD ON COND\n");
    map<thrID, bool> threadIsOnCond;
    map<thrID, pthread_cond_t *>::iterator itr;
    for (itr = s->cond_map.begin(); itr != s->cond_map.end(); itr++) {
        if (itr->second == s->cond) {
           threadIsOnCond[itr->first] = true; 
        } else {
           threadIsOnCond[itr->first] = false; 
        }
    }
 
    thrID thread = chooseRandomThread(threadIsOnCond.begin(),
            threadIsOnCond.end());
    if (thread != -1) {
        safe_assert(s->cond_map[thread] == s->cond);
    }
    return thread;
}
