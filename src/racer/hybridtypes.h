/**
 * hybridtypes.h - Typedefs for hybrid race tracking 
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */

#ifndef _HYBRIDTYPES_H_
#define _HYBRIDTYPES_H_

#include <map>
#include <set>
#include <vector>
#include <fstream>

using namespace std;

typedef int thrID;
typedef map<thrID, int> vectorClock;
typedef map<thrID, int> messageMap;
typedef map<thrID, vectorClock> vcContainer;
typedef map<pthread_cond_t *, messageMap> msgContainer;
typedef set<pthread_mutex_t *> lockSet;
typedef map<thrID, lockSet> lsContainer;

struct raceEvent {
    thrID thread;
    void * addr;
    bool isread;
    vectorClock vc;
    lockSet ls;

    raceEvent(thrID thethread,
                void * theaddr,
                bool theisread,
                vectorClock thevc,
                lockSet thels) {
        thread = thethread;
        addr = theaddr;
        isread = theisread;
        vc = thevc;
        ls = thels;
    }

    bool operator==(const raceEvent & other) const {
        if (other.thread != thread) 
            return false;
        if (other.addr != addr)
            return false;
        if (other.isread != isread)
            return false;
        if (other.vc != vc)
            return false;
        if (other.ls != ls) 
            return false;
        return true;
    }

    bool operator!=(const raceEvent & other) const {
        return !(*this == other);
    }

};

struct dataRaceEvent: public raceEvent {
    void * iid;

    dataRaceEvent(void * _iid, thrID thr, void * addr, bool read,
            vectorClock vc, lockSet ls) : raceEvent(thr, addr, read, vc, ls) {
        iid = _iid;
    }

    //TODO: this right now is used for set inclusion, come
    //          up with a more sensical < operator
    bool operator< (const dataRaceEvent & c) const {
        if (iid != c.iid)
            return (iid < c.iid);

        return (thread < c.thread);
    }

    bool operator==(const dataRaceEvent & other) const {
        if (other.iid != iid) 
            return false;
        if (other.thread != thread) 
            return false;
        if (other.addr != addr)
            return false;
        if (other.isread != isread)
            return false;
        if (other.vc != vc)
            return false;
        if (other.ls != ls) 
            return false;
        return true;
    }

    bool operator!=(const dataRaceEvent & other) const {
        return !(*this == other);
    }

    friend ostream& operator<<(ostream & output, const dataRaceEvent & ev);

};

struct lockRaceEvent: public raceEvent {
    void * lock;

    lockRaceEvent(void * _lock, thrID thr, void * addr, bool read,
            vectorClock vc, lockSet ls) : raceEvent(thr, addr, read, vc, ls) {
        lock = _lock;
    }
};

class raceRecord {
    public:
        raceRecord() {}

};

class dataRaceRecord : public raceRecord{
    void * iidleft;
    void * iidright;

    public: 
    dataRaceRecord(void * iidl, void * iidr) : raceRecord() {
        if (iidl < iidr) {
            iidleft = iidl;
            iidright = iidr;
        } else {
            iidleft = iidr;
            iidright = iidl;
        }
    }

    void * getLeft() {
        return iidleft;
    }

    void * getRight() {
        return iidright;
    }


    friend bool operator== (dataRaceRecord & rr1, dataRaceRecord & rr2);
    friend bool operator!= (dataRaceRecord & rr1, dataRaceRecord & rr2);
    friend ostream& operator<<(ostream & output, const dataRaceRecord & rr);

    bool operator< (const dataRaceRecord & c) const {
        if (iidleft == c.iidleft) {
           return iidright < c.iidright;
        } else {
           return iidleft < c.iidleft;
        } 
    }
};

class lockRaceRecord : public raceRecord {
    void * idleft;
    void * idright;

    public:
    lockRaceRecord(void * id1, void * id2) : raceRecord() {
        if (id1 < id2) {
            idleft = id1;
            idright = id2;
        } else {
            idright = id1;
            idleft = id2;
        }
    }

    void * getLeft() {
        return idleft;
    }

    void * getRight() {
        return idright;
    }

    friend bool operator== (lockRaceRecord & rr1, lockRaceRecord & rr2);
    friend bool operator!= (lockRaceRecord & rr1, lockRaceRecord & rr2);
    friend ostream& operator<<(ostream & output, const lockRaceRecord & rr);

};



#endif
