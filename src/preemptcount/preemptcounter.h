/**
 * preemptcounter - calculates the statistics of an execution (preemptions,
 * blocking threads etc)
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _PREEMPTCOUNTER_H_
#define _PREEMPTCOUNTER_H_

#include "../randomschedule/randomtracker.h"
#include "fairschedule.h"


class PreemptionCounter: public RandomTracker {
    private:
        int preemption_budget;
        int my_preempts;
        int thread_blocks;
        int context_switches;
        int scheduling_points;
        int threads_to_explore;
        FairScheduler fs;

    public:
        PreemptionCounter(thrID myself);
        virtual ~PreemptionCounter();

    protected:
        virtual bool insertPreemption(thrID, map<thrID, bool>);
        virtual thrID schedule(thrID);
        virtual thrID pickNextFairChoice(map<thrID, bool>);
        friend class PreemptionCounterTestSuite;

        virtual unsigned int SimulateSleep(thrID, unsigned int);
        virtual int SimulateUsleep(thrID, useconds_t);
        virtual int SimulateCondTimedwait(thrID,
                pthread_cond_t *, 
                pthread_mutex_t *,
                const struct timespec *,
                void *);



};


#endif

