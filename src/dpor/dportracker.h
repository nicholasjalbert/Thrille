/**
 * dportracker - tracks the status of a program's execution, random schedules,  
 *  outputs information to allow DPOR style model checking
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _DPORTRACKER_H_
#define _DPORTRACKER_H_

#include "../randomschedule/randomtracker.h"
#include "../racedetect/vctrack.h"
#include <vector>

typedef int transitionID;


class DporTracker: public RandomTracker {
    public:

        DporTracker(thrID myself) 
            : RandomTracker(myself) {
                printf("DPOR tracker ots\n");
                currentTransition = 0;
                vc = new VectorClockTracker();
            }

    protected:
        transitionID currentTransition;
        map<pthread_mutex_t *, transitionID> last_lock_access_map;
        map<pthread_mutex_t *, vectorClock> last_lock_access_vc;
        map<void *, transitionID> last_mem_access_map;
        map<void *, vectorClock> last_mem_access_vc;
        map<thrID, void *> current_process_state;

        vector<thrID> transitions_chosen;
        vector<vector<thrID> > backtrack_set;
        vector<vector<thrID> > done_set;
        VectorClockTracker * vc;


        virtual void incrementCurrentTransition();
        virtual transitionID getCurrentTransition();
        virtual void dporThreadStart(thrID, void *); 
        virtual void dporThreadFinish(thrID, void *); 
        virtual void dporBeforeCondTimedwait(thrID, 
                pthread_cond_t *, 
                pthread_mutex_t *,
                const struct timespec *);
        virtual void dporAfterCondTimedwait(thrID,
                pthread_cond_t *, 
                pthread_mutex_t *,
                const struct timespec *);
        virtual void dporBeforeCreate(thrID,
                thrID,
                pthread_t *, 
                const pthread_attr_t *,
                void *(*)(void *),
                void *,
                ThreadInfo * &);
        virtual void dporAfterCreate(thrID,
                int,
                pthread_t *,
                const pthread_attr_t *,
                void *(*)(void *),
                void * );
        virtual void dporAfterJoin(thrID, thrID, pthread_t, void**);
        virtual void dporBeforeMutexLock(thrID, pthread_mutex_t *);
        virtual void dporSimulateMutexLock(thrID, pthread_mutex_t *);
        virtual void dporAfterMutexUnlock(thrID, int, pthread_mutex_t *);
        virtual void dporBeforeCondWait(thrID, 
                pthread_cond_t *, 
                pthread_mutex_t *);
        virtual void dporAfterCondWait(thrID,
                pthread_cond_t *, 
                pthread_mutex_t *);
        virtual void dporBeforeCondSignal(thrID, pthread_cond_t *);
        virtual void dporSimulateCondSignal(thrID, pthread_cond_t *);
        virtual void dporBeforeCondBroadcast(thrID, pthread_cond_t *); 
        virtual void dporSimulateCondBroadcast(thrID, pthread_cond_t *);
        virtual void dpormyMemoryRead(thrID, int, void *);
        virtual void dpormyMemoryWrite(thrID, int, void *);
        virtual void dporBeforeExit(thrID, void *);
        virtual void dporSimulateExit(thrID, void *);
        virtual void dpormyUsleep(thrID, useconds_t);

};


#endif

