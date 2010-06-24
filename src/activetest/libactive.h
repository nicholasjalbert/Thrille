/**
* libactive - Pthreads active testing implementation 
* 
* Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
*
* <Legal matter>
*/

#ifndef _LIBACTIVE_H_
#define _LIBACTIVE_H_

#include "../thrille-core/libpth.h"
#include "activetypes.h"

class ActiveHandler : public Handler {
    private: 
        pthread_mutex_t *  my_mutex;
        pthread_cond_t * my_cond;
        int msTimeout;
        int firstiid;
        int secondiid;
        bool raceDiscovered;
        bool isFirst;
        set<raceEvent *> eventSet;

    public:
        ActiveHandler() : Handler() {
            dbgPrint("Starting Active Random Testing\n");
            msTimeout = 300;
            raceDiscovered = false;
            my_mutex = new pthread_mutex_t;
            my_cond = new pthread_cond_t;
            Originals::pthread_mutex_init(my_mutex, NULL);
            Originals::pthread_cond_init(my_cond, NULL);
        }

        virtual ~ActiveHandler() {
            dbgPrint("Ending Active Random Testing...\n");
            if (raceDiscovered)
                dbgPrint("***Race Discovered between %d and %d***\n",
                        firstiid,
                        secondiid);
            pthread_mutex_destroy(my_mutex);
            pthread_cond_destroy(my_cond);
        }



    protected:
        void getTargetIIDs();
        void AfterInitialize();
        void initHandler();        
        void myMemoryRead(int iid, void * addr);
        void myMemoryWrite(int iid, void * addr);
        void prepTimedWait(timespec *);
        bool checkRace(raceEvent);
        void addEvent(raceEvent *);
        void removeEvent(raceEvent *);
        friend void myMemoryRead(int, void*);
        friend void myMemoryWrite(int, void*);
};

#endif
