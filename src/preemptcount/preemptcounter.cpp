
#define UNLOCKASSERT
#include "preemptcounter.h"

PreemptionCounter::PreemptionCounter(thrID myself) : RandomTracker(myself) {
    preemption_budget = 2;
    my_preempts = 0;
    thread_blocks = 0;
    context_switches = 0;
    scheduling_points = 0;
    threads_to_explore = 0;

}

PreemptionCounter::~PreemptionCounter() {
    safe_assert(context_switches == my_preempts + thread_blocks);
    safe_assert(my_preempts <= preemption_budget);
    safe_assert(scheduling_points >= context_switches);
    printf("Run Statistics (Fair Scheduling Enabled):\n");
    printf("\tPreemption Budget: %d\n", preemption_budget);
    printf("\tPreemptive Context Switches: %d\n", my_preempts);
    printf("\tNon-Preemptive Context Switches: %d\n", thread_blocks);
    printf("\tThreads to Explore at Non-Preemptive CS: %d\n",
            threads_to_explore);
    printf("\tTotal Context Switches: %d\n", context_switches);
    printf("\tScheduling Points: %d\n", scheduling_points);
}


thrID PreemptionCounter::pickNextFairChoice(map<thrID, bool> fairenmap) {
    thrID thread = chooseRandomThread(fairenmap.begin(), fairenmap.end());
    if (thread == -1) {
        printf("Deadlock Discovered...\n");
        safe_assert(false);
        return -1;
    } else {
        safe_assert(enable_map[thread]);
        safe_assert(fairenmap[thread]);
        return thread;
    }
}

thrID PreemptionCounter::schedule(thrID myself) {
    scheduling_points++;
    thrID nextchoice = log->getNextChoice();
    if (nextchoice == -1) {
        map<thrID, bool> fairenmap;
        map<thrID, bool>::iterator itr;
        /*
        printf("fair enabled map:\n");
        for (itr = fairenmap.begin(); itr != fairenmap.end(); itr++) {
            if (itr->second)
                printf("\t%d: true\n", itr->first);
            else
                printf("\t%d: false\n", itr->first);
        }*/
        fairenmap = fs.getFairEnabledMap(enable_map);

        if (fs.isFairEnabled(myself, enable_map)) {
            if (insertPreemption(myself, fairenmap)) {
                nextchoice = myself;
                my_preempts++;
                context_switches++;
                while (nextchoice == myself) {
                    nextchoice = this->pickNextFairChoice(fairenmap);
                }
                for (itr = fairenmap.begin(); itr != fairenmap.end(); itr++) {
                    if (itr->second) {
                        threads_to_explore++;
                    }
                }
            } else {
                nextchoice = myself;
            }
        } else {
            for (itr = fairenmap.begin(); itr != fairenmap.end(); itr++) {
                if (itr->second) {
                    threads_to_explore++;
                }
            }
            thread_blocks++;
            context_switches++;
            nextchoice = this->pickNextFairChoice(fairenmap);
        }
        fs.fairChosen(nextchoice);
        launchThread(nextchoice);
        return nextchoice;
    } else {
        if (nextchoice != myself) {
            context_switches++;
            if (enable_map[myself]) {
                my_preempts++;
            } else {
                thread_blocks++;
            }
        } 
        launchThread(nextchoice);
        return nextchoice; 
    }
}

bool PreemptionCounter::insertPreemption(thrID myself,
        map<thrID, bool> fairenmap) {
    safe_assert(fairenmap[myself]);
    if (my_preempts >= preemption_budget) {
        return false;
    }
    int enable_count = 0;
    map<thrID, bool>::iterator itr;
    for (itr = fairenmap.begin(); itr != fairenmap.end(); itr++) {
        if (itr->second) {
            enable_count++;
        }
    }
    if (enable_count < 2) {
        return false;
    }

    int CHANCE_OF_PREEMPTION = 2000; //1 in X chance of preemption
    int choice = rand() % CHANCE_OF_PREEMPTION;
    return (choice == 0);
}

unsigned int PreemptionCounter::SimulateSleep(thrID myself, unsigned int) {
    glock();
    log->mySleep(myself);
    handleMySleep(myself);
    schedule(myself);
    gunlock();
    pauseThread(myself);
    fs.lastWasYield();
    return 0;
}

int PreemptionCounter::SimulateUsleep(thrID myself, useconds_t) {
    glock();
    log->myUsleep(myself);
    handleMyUsleep(myself);
    schedule(myself);
    gunlock();
    pauseThread(myself);
    fs.lastWasYield();
    return 0;
}

int PreemptionCounter::SimulateCondTimedwait(thrID myself,
                pthread_cond_t * cond, 
                pthread_mutex_t * lock,
                const struct timespec * time,
                void * id) {
    glock();
    log->SimulateCondTimedwait(myself);

    //unlock
    bool proceed = BeforeMutexUnlock(myself, lock, true);
    safe_assert(proceed);
    safe_assert(Originals::pthread_mutex_unlock(lock) == 0);
    AfterMutexUnlock(myself, 0, lock, true);
    
    
    cond_map[myself] = cond;
    handleSimulateCondTimedwait(myself, cond, lock, id);
    schedule(myself);
    gunlock();

    waitPauseThread(myself);

    glock();
    cond_map[myself] = NULL;
    gunlock();

    //lock
    proceed = BeforeMutexLock(myself, lock, id, true);
    safe_assert(proceed == false);
    signalThreadReady(myself);
    glock();
    bool was_signalled = was_signalled_map[myself];
    if (! was_signalled) {
        schedule(myself);
        was_signalled_map[myself] = true;
    }
    gunlock();
    pauseThread(myself);
    fs.lastWasYield();
    safe_assert(SimulateMutexLock(myself, lock, true) == 0);
    AfterMutexLock(myself, 0, lock, true);
    if (was_signalled) {
        log->timedwaitNoTimeout(myself);
        glock();
        gunlock();
        return 0;
    } else {
        log->timedwaitTimeout(myself);
        glock();
        gunlock();
        return ETIMEDOUT;        
    }

}

