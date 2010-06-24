#include "hybridtrack.h"

HybridRaceTracker::HybridRaceTracker() {
}


HybridRaceTracker::HybridRaceTracker(unsigned int wr, unsigned int rd) {
    wr_profile_size = wr;
    rd_profile_size = rd;
} 

HybridRaceTracker::~HybridRaceTracker() {
}

unsigned int HybridRaceTracker::getWrProfileSize() {
    return wr_profile_size;
}

unsigned int HybridRaceTracker::getRdProfileSize() {
    return rd_profile_size;
}


void HybridRaceTracker::checkRace(dataRaceEvent e) {
    vector<dataRaceEvent> addrevents = eventset[e.addr];
    vector<dataRaceEvent>::iterator itr;
    for (itr = addrevents.begin(); itr != addrevents.end(); itr++) {
        if (isRacing(e, (*itr))) {
            dataRaceRecord tmp(e.iid, itr->iid);
            vector<dataRaceRecord>::iterator vitr;
            bool addrace = true;
            for (vitr = foundraces.begin(); vitr != foundraces.end(); vitr++) {
                if ((*vitr) == tmp) {
                    addrace = false;
                }
            }
            if (addrace) { 
                foundraces.push_back(tmp);
            }
        }
    }
}

void HybridRaceTracker::addEvent(dataRaceEvent e) {
    vector<dataRaceEvent> eset = eventset[e.addr];
    if (e.isread) {
        if (eset.size() < rd_profile_size) {
            eset.push_back(e);
        }
    } else {
        if (eset.size() < rd_profile_size) {
            eset.push_back(e);
        }
    }
    eventset[e.addr] = eset;
}

bool HybridRaceTracker::isRacing(raceEvent e1, raceEvent e2) {
    if (e1.thread == e2.thread) {
        return false;
    }
    if (e1.addr != e2.addr) {
        return false;
    }
    if (e1.isread && e2.isread) {
        return false;
    }
    if (vsTracker.isHappensBefore(e1.vc, e2.vc)) {
        return false;
    }
    if (vsTracker.isHappensBefore(e2.vc, e1.vc)) {
        return false;
    }
    if (! lsTracker.isIntersectionEmpty(e1.ls, e2.ls)) {
        return false;
    }

    return true;
}


void HybridRaceTracker::afterJoin(thrID me, thrID thrjoined) {
    vsTracker.afterJoin(me, thrjoined);
}

void HybridRaceTracker::beforeSignal(thrID me, pthread_cond_t * cond) {
    vsTracker.beforeSignal(me, cond);
}

void HybridRaceTracker::afterWait(thrID me, pthread_cond_t * cond) {
    vsTracker.afterWait(me, cond);
}

void HybridRaceTracker::beforeCreate(thrID me, thrID child) {
    vsTracker.beforeCreate(me, child);
}

void HybridRaceTracker::lockBefore(thrID thread, pthread_mutex_t * lock) {
    lsTracker.lockBefore(thread, lock);
}

void HybridRaceTracker::unlockAfter (thrID thread, pthread_mutex_t * lock) {
    lsTracker.unlockAfter(thread, lock);
}

lockSet HybridRaceTracker::getLockSet(thrID thread) {
    return lsTracker.getLockSet(thread);
}

vectorClock HybridRaceTracker::getVectorClock(thrID thread) {
    return vsTracker.getVectorClock(thread);
}

vector<dataRaceRecord> HybridRaceTracker::getRaces() {
    return foundraces;
}

map<void *, vector<dataRaceEvent> > HybridRaceTracker::getRaceEvents() {
    return eventset;
} 

void HybridRaceTracker::dumpRaces(ofstream * fout) {
    vector<dataRaceRecord>::iterator itr;
    if (((int) foundraces.size()) == 0) {
        printf("No races discovered\n");
    }
    for (itr = foundraces.begin(); itr != foundraces.end(); itr++) {
        printf("Race: RaceRecord(%p, %p)\n", 
                ((dataRaceRecord)(*itr)).getLeft(),       
                ((dataRaceRecord)(*itr)).getRight());
        (*fout) << ((dataRaceRecord)(*itr)).getLeft() << "," 
            << ((dataRaceRecord) (*itr)).getRight()
            << endl << flush; 
    }
    int racecount = 0; 
    map<void *, vector<dataRaceEvent> > potraces = getRaceEvents();
    map<void *, vector<dataRaceEvent> >::iterator mitr;
    vector<dataRaceEvent>::iterator sitr;
    vector<dataRaceEvent> tmp;
    for (mitr = potraces.begin(); mitr != potraces.end(); mitr++) {
        tmp = mitr->second; 
        for (sitr = tmp.begin(); sitr != tmp.end(); sitr++) {
            racecount++;
        }
    }
    printf("\nRace Events: %d\n", racecount);
}

void HybridRaceTracker::outputRaces() {
    map<void *, vector<dataRaceEvent> >::iterator mitr;
    vector<dataRaceEvent>::iterator vitr;
    for (mitr = eventset.begin(); mitr != eventset.end(); mitr++) {
        vector<dataRaceEvent> tmpv = mitr->second;
        for (vitr = tmpv.begin(); vitr != tmpv.end(); vitr++) {
            checkRace((*vitr));
        }
    }
    ofstream * fout = new ofstream;
    fout->open("races.log");
    dumpRaces(fout);
    fout->close();
    delete fout;

}

void HybridRaceTracker::addLockEvent(lockRaceEvent) {
    
}

ostream& operator<<(ostream & output, const dataRaceEvent & ev) {
    output << "RaceEvent (\n\tline: " << ev.iid << "\n\tthr: " << 
        ev.thread << "\n\taddr: " << ev.addr<< "\n\tisRead: " <<
        ev.isread << "\n\tVC: ["; 

    vectorClock::iterator itr;
    vectorClock testclock = ev.vc;
    for (itr = testclock.begin(); itr != testclock.end(); itr++) {
        output << itr->first << ":" << itr->second << ","; 
    }
    output << "]\n\tLock:[";

    lockSet::iterator litr;
    for (litr = ev.ls.begin(); litr != ev.ls.end(); litr++) {
        output << (void *) (* litr) << ",";
    }

    output << "]\n)" << endl;
    return output;
}

ostream& operator<<(ostream & output, const dataRaceRecord & rr) {
    output << "RaceRecord(" << rr.iidleft << "," << rr.iidright 
        << ")" << endl;
    return output;
}

    bool operator== (dataRaceRecord & rr1, dataRaceRecord & rr2) {
        if (rr1.iidleft == rr2.iidleft && rr1.iidright == rr2.iidright)
            return true;
        if (rr1.iidleft == rr2.iidright && rr1.iidright == rr2.iidleft)
            return true;
        return false;
    }

bool operator!= (dataRaceRecord & rr1, dataRaceRecord & rr2) {
    return !(rr1 == rr1);
}


ostream& operator<<(ostream & output, const lockRaceRecord & rr) {
    output << "LockRaceRecord(" << rr.idleft << "," << rr.idright 
        << ")" << endl;
    return output;
}

    bool operator== (lockRaceRecord & rr1, lockRaceRecord & rr2) {
        if (rr1.idleft == rr2.idleft && rr1.idright == rr2.idright)
            return true;
        if (rr1.idleft == rr2.idright && rr1.idright == rr2.idleft)
            return true;
        return false;
    }

bool operator!= (lockRaceRecord & rr1, lockRaceRecord & rr2) {
    return !(rr1 == rr1);
}




