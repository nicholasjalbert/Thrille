/**
 * threadtracker - class for tracking thread id's, replaces old
 *                 thread specific data method
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */

#ifndef _THREADTRACKER_H_
#define _THREADTRACKER_H_

#include "coretypes.h"
#include "originals.h"

using namespace std;

class ThreadTracker {
    private:
        thrID curr_id;
        map<thrID, pthread_t> pthreadHandleMap;
        pthread_mutex_t threadtracker_mutex;

    public:
        ThreadTracker();
        virtual ~ThreadTracker();

        virtual thrID getNextTID();
        virtual void setTID(thrID, pthread_t);
        virtual thrID translateHandleToTID(pthread_t);

        friend class ThreadTrackerTestSuite;
};

#endif
