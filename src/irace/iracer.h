/**
* iracer - [purpose] 
* 
* Author -  [name] [email]
*
* <Legal matter>
*/

#ifndef _IRACER_H
#define _IRACER_H

#include "../serializer/racer.h"
#include "iracetypes.h"

struct LockInfo {
    thrID thread;
    vectorClock vc;

    LockInfo() {
        thread = -1;
        vc = vectorClock();
    }

    LockInfo(thrID thr, vectorClock v) {
        thread = thr;
        vc = v;
    }
};

struct RaceInfo {
    thrID thread;
    int iid;
    vectorClock vc;
    bool iswrite;

    RaceInfo() {
        vc = vectorClock();
        iid = -1;
        thread = -1;
        iswrite = false;

    }

    RaceInfo(thrID thr, int myiid, vectorClock myvc, bool wr) {
        vc = myvc;
        iid = myiid;
        thread = thr;
        iswrite = wr;
    }

};

struct LockRaceEvent {
    thrID thread1;
    thrID thread2;
    pthread_mutex_t * lock;

    LockRaceEvent(thrID thr1, thrID thr2, pthread_mutex_t * l) {
        thread1 = thr1;
        thread2 = thr2;
        lock = l;
    }

    friend bool operator==(LockRaceEvent a, LockRaceEvent b);
};

struct RaceEvent {
    thrID thread1;
    thrID thread2;
    int iid1;
    int iid2;
    bool wasWrite1;
    bool wasWrite2;
    void * addr;

    RaceEvent(thrID thr1, thrID thr2, int id1, int id2, 
            bool wr1, bool wr2, void * ad) {

        thread1 = thr1;
        thread2 = thr2;
        iid1 = id1;
        iid2 = id2;
        wasWrite1 = wr1;
        wasWrite2 = wr2;
        addr = ad;
    }

    friend bool operator==(RaceEvent a, RaceEvent b);

};



class Iracer : public Racer {
    private:
        map<thrID, vectorClock> vc;
        map<pthread_mutex_t *, vectorClock> lockmap;
        map<pthread_mutex_t *, LockInfo> acq_lockmap;
        map<pthread_cond_t *, vectorClock> condmap;
        map<void *, map<thrID, RaceInfo> > accessmap;
        map<void *, RaceInfo> writemap;
        vector<RaceEvent> races;
        vector<LockRaceEvent> lockraces;

    protected:
        virtual void incrementMyClock(thrID);
        virtual void donateClock(thrID, thrID);
        virtual void dumpRaces();
        virtual void dumpLockRaces();

    public:
        Iracer(thrID main) : Racer(main){
            printf("Immediate race detection started...\n");
            incrementMyClock(main);
        }

        virtual ~Iracer() {
            dumpRaces();
            dumpLockRaces();
        }

        virtual void ThreadStart(thrID); 
        virtual void ThreadFinish(thrID); 
        virtual void BeforeCreate(thrID, thrID); 
        virtual void AfterJoin(thrID, thrID);
        virtual void AfterMutexLock(thrID, pthread_mutex_t *);
        virtual void BeforeMutexUnlock(thrID, pthread_mutex_t *);
        virtual void BeforeCondSignal(thrID, pthread_cond_t *);
        virtual void BeforeCondBroadcast(thrID, pthread_cond_t *);
        virtual void AfterCondWait(thrID, pthread_cond_t *);
        virtual void AfterCondTimedwait(thrID, int, pthread_cond_t *); 
        virtual void memoryRead(thrID, int, void *);
        virtual void memoryWrite(thrID, int, void *);

        virtual void printVC(thrID);
        virtual void startNewThread(thrID);
        virtual bool maxMyClock(thrID, vectorClock);
        virtual bool checkWriteRace(thrID, int, void *);
        virtual bool checkReadRace(thrID, int, void *);
        virtual bool isLockRace(vectorClock, vectorClock);
        virtual void zeroAccessMapVC(void *, thrID);
};

#endif
