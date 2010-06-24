/**
* barriertracker - class for tracking barrier counts etc
* 
* Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
*
* <Legal matter>
*/

#ifndef _BARRIERTRACKER_H_
#define _BARRIERTRACKER_H_

#include "serializertypes.h"

struct BarrierInfo {
    unsigned count;
    pthread_barrier_t * barrier;

    BarrierInfo() {
        count = -1;
        barrier = NULL;
    }

    BarrierInfo(pthread_barrier_t * bar, unsigned num) {
        barrier = bar;
        count = num;
    }
};

class BarrierTracker {
    private:
        map<thrID, pthread_barrier_t *> thread_map;
        vector<BarrierInfo> bar_info;

    public:
        BarrierTracker();
        virtual ~BarrierTracker();
        virtual void initBarrier(pthread_barrier_t *, unsigned);
        virtual unsigned getBarrierCount(pthread_barrier_t *);
        virtual void threadWaitingOnBarrier(thrID, pthread_barrier_t *);
        virtual int getNumberThreadsWaitingOnBarrier(pthread_barrier_t *);
        virtual vector<thrID> getThreadsWaitingOnBarrier(pthread_barrier_t *);
        virtual void clearThreadsWaitingOnBarrier(pthread_barrier_t *);

};

#endif
