#define UNLOCKASSERT
#include "randomtracker.h"
        
RandomTracker::RandomTracker(thrID myself) : ExecutionTracker(myself) {
    srand(time(0));
    chanceOfPreempt = 1;
    printf("Thrille: Randomscheduler now defaults to scheduling at ");
    printf("*EVERY* memory access\n");
    printf("Thrille: Chance of preemption set at 1/%d\n", chanceOfPreempt);
    log->scheduleAtAllMemory();
}

RandomTracker::~RandomTracker() {
    
}



thrID RandomTracker::pickNextSchedulingChoice(SchedPointInfo * s) {
    if (enable_map[s->thread]) {

        int preempt = log->getRandomNumber() % chanceOfPreempt;

        if (preempt != 0) {
            safe_assert(enable_map[s->thread]);
            return s->thread;
        }
    }

    thrID thread = chooseRandomThread(enable_map.begin(), enable_map.end());
    if (thread == INVALID_THREAD) {
        deadlockCheck();
        return INVALID_THREAD;
    } else {
        safe_assert(enable_map[thread]);
        return thread;
    }
}

thrID RandomTracker::selectThreadWaitingOnCond(SignalPointInfo * s) {
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
    if (thread != INVALID_THREAD) {
        safe_assert(s->cond_map[thread] == s->cond);
    }
    return thread;
}

thrID RandomTracker::chooseRandomThread(map<thrID, bool>::iterator begin,
        map<thrID, bool>::iterator end) {

    map<thrID, bool>::iterator itr;

    int numberOfChoices= 0;
    for (itr = begin; itr != end; itr++) {
        if (itr->second) {
            numberOfChoices++;
        }
    }

    if (numberOfChoices == 0) {
        return INVALID_THREAD;
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
    safe_assert(false);
    return INVALID_THREAD;
}
