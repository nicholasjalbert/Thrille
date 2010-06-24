#include "iracer.h"

void Iracer::ThreadStart(thrID myself) {
    incrementMyClock(myself);
    //printf("Just Started %d\n", myself);
    printVC(myself);
}

void Iracer::ThreadFinish(thrID myself) {
    incrementMyClock(myself);
    //printf("Just Finishing %d\n", myself);
    printVC(myself);
}

void Iracer::BeforeCreate(thrID myself, thrID child) {
    startNewThread(child);
    donateClock(myself, child);
    incrementMyClock(myself);
    printf("Just before create %d\n", myself);
    printVC(myself);

}

void Iracer::AfterJoin(thrID myself, thrID target) {
    maxMyClock(myself, vc[target]);
    incrementMyClock(myself);
    printf("Just After Thread %d joining Thread %d\n", myself, target);
    printVC(myself);
}

void Iracer::AfterMutexLock(thrID myself, pthread_mutex_t * lock) {
    bool lockrace = isLockRace(vc[myself], acq_lockmap[lock].vc); 
    if (lockrace) {
        printf("Lock Race Found between thread %d and thread %d\n", 
                acq_lockmap[lock].thread, myself);
        LockRaceEvent lrace(acq_lockmap[lock].thread, myself, lock);
        lockraces.push_back(lrace);
    }
    LockInfo tmp(myself, vc[myself]);
    acq_lockmap[lock] = tmp;
    maxMyClock(myself, lockmap[lock]);
    incrementMyClock(myself);
    printf("After lock in thread %d\n", myself);
    printVC(myself);
}

void Iracer::BeforeMutexUnlock(thrID myself, pthread_mutex_t * lock) {
    lockmap[lock] = vc[myself];
    incrementMyClock(myself);
    printf("Before unlock in thread %d\n", myself);
    printVC(myself);
}

void Iracer::BeforeCondSignal(thrID myself, pthread_cond_t * cond) {
    condmap[cond] = vc[myself];
    incrementMyClock(myself);
    printf("Before signal in thread %d\n", myself);
    printVC(myself);
}

void Iracer::BeforeCondBroadcast(thrID myself, pthread_cond_t * cond) {
    condmap[cond] = vc[myself];
    incrementMyClock(myself);
    printf("Before broadcast in thread %d\n", myself);
    printVC(myself);
}

void Iracer::AfterCondWait(thrID myself, pthread_cond_t * cond) {
    maxMyClock(myself, condmap[cond]);
    incrementMyClock(myself);
    printf("After wait in thread %d\n", myself);
    printVC(myself);
}

void Iracer::AfterCondTimedwait(thrID myself, int ret, pthread_cond_t * cond) {
    if (ret == 0) {
        maxMyClock(myself, condmap[cond]);
        incrementMyClock(myself);
        printf("After (signalled) timedwait in thread %d\n", myself);
        printVC(myself);
    }  
}

void Iracer::memoryRead(thrID myself, int iid, void * addr) {
    bool israce = checkReadRace(myself, iid, addr);
    RaceInfo myinfo(myself, iid, vc[myself], false);
    accessmap[addr][myself] = myinfo;
    incrementMyClock(myself);
    printf("Thread %d memory read\n", myself);
    if (israce) {
        printf("RACE FOUND\n");
    }
    printVC(myself);
}

void Iracer::memoryWrite(thrID myself, int iid, void * addr) {
    bool israce = checkWriteRace(myself, iid, addr);
    RaceInfo myinfo(myself, iid, vc[myself], true);
    accessmap[addr][myself] = myinfo;
    writemap[addr] = myinfo;
    incrementMyClock(myself);
    printf("Thread %d memory write\n", myself);
    if (israce) {
        printf("RACE FOUND\n");
    }
    printVC(myself);
}

void Iracer::incrementMyClock(thrID myself) {
    vectorClock myclock = vc[myself];
    int mytime = myclock[myself];
    mytime++;
    myclock[myself] = mytime;
    vc[myself] = myclock;
}

void Iracer::donateClock(thrID myself, thrID target) {
    vectorClock myclock = vc[myself];
    vectorClock::iterator itr;
    for (itr = myclock.begin(); itr != myclock.end(); itr++) {
        vc[target][itr->first] = itr->second;
    }
}

void Iracer::printVC(thrID myself) {
    return;
    vectorClock myclock = vc[myself];
    vectorClock::iterator itr;
    printf("Thread %d's clock:\n", myself);
    for (itr = myclock.begin(); itr != myclock.end(); itr++) {
        printf("\t %d : %d\n", itr->first, itr->second);
    }
}

void Iracer::startNewThread(thrID child) {
    map<thrID, vectorClock>::iterator itr;
    for (itr = vc.begin(); itr != vc.end(); itr++) {
        (itr->second)[child] = 0;
    }
}


bool Iracer::maxMyClock(thrID myself, vectorClock maxvc) {
    vectorClock myclock = vc[myself];
    vectorClock::iterator itr;
    bool ret_val = false;
    for (itr = myclock.begin(); itr != myclock.end(); itr++) {
        int mine = itr->second;
        int other = maxvc[itr->first];
        if (mine < other) {
            ret_val = true;
            myclock[itr->first] = other;
        } else {
            myclock[itr->first] = mine;
        }
    }
    vc[myself] = myclock;
    return ret_val;
}

bool Iracer::checkWriteRace(thrID myself, int iid, void * addr) {
    map<thrID, RaceInfo> accmap = accessmap[addr];
    map<thrID, RaceInfo>::iterator itr;
    bool israce = false;
    bool anyrace = false;
    for (itr = accmap.begin(); itr != accmap.end(); itr++) {
        israce = maxMyClock(myself, itr->second.vc);
        anyrace = anyrace || israce;
        if (israce) {
            RaceEvent tmp(myself, itr->first, iid,
                    itr->second.iid, true, itr->second.iswrite, addr);
            races.push_back(tmp);
        }
    }
    map<thrID, RaceInfo> newmap;
    accessmap[addr] = newmap;
    return anyrace;
}

bool Iracer::checkReadRace(thrID myself, int iid, void * addr) {
    RaceInfo compare = writemap[addr];
    bool israce = maxMyClock(myself, compare.vc);
    if (israce) {
        RaceEvent tmp(myself, compare.thread, iid,
                compare.iid, false, compare.iswrite, addr);
        races.push_back(tmp);
    }
    zeroAccessMapVC(addr, compare.thread);
    return israce;
}

void Iracer::zeroAccessMapVC(void * addr, thrID target) {
    accessmap[addr][target] = RaceInfo();
}

void Iracer::dumpRaces() {
    vector<RaceEvent> result;
    vector<RaceEvent>::iterator itr;
    vector<RaceEvent>::iterator sitr;
    for (itr = races.begin(); itr != races.end(); itr++) {
        RaceEvent tmp = (*itr);
        bool isunique = true;
        for (sitr = result.begin(); sitr != result.end(); sitr++) {
            if (tmp == (*sitr)) {
                isunique = false;
                break;
            }
        } 
        if (isunique) {
            result.push_back(tmp);
        }
    }
    printf("\n--------------------------\n");
    int totalraces = 0;
    for (itr = result.begin(); itr != result.end(); itr++) {
        RaceEvent tmp = (*itr);
        totalraces++;
        printf("Race found:\n");
        printf("\tThr1: %d, Thr2: %d\n", tmp.thread1, tmp.thread2);
        printf("\tiid1: %d, iid2: %d\n", tmp.iid1, tmp.iid2);
        printf("\twr1: %d, wr2: %d\n", tmp.wasWrite1, tmp.wasWrite2);
        printf("\taddress: %p\n-------------\n", tmp.addr);
    }
    printf("\n--------------------------\n");
    printf("Total Races: %d\n", totalraces);

}

void Iracer::dumpLockRaces() {
    vector<LockRaceEvent> result;
    vector<LockRaceEvent>::iterator itr;
    vector<LockRaceEvent>::iterator sitr;
    for (itr = lockraces.begin(); itr != lockraces.end(); itr++) {
        LockRaceEvent tmp = (*itr);
        bool isunique = true;
        for (sitr = result.begin(); sitr != result.end(); sitr++) {
            if (tmp == (*sitr)) {
                isunique = false;
                break;
            }
        } 
        if (isunique) {
            result.push_back(tmp);
        }
    }
    printf("\n--------------------------\n");
    int totalraces = 0;
    for (itr = result.begin(); itr != result.end(); itr++) {
        LockRaceEvent tmp = (*itr);
        totalraces++;
        printf("Lock Race found:\n");
        printf("\tThr1: %d, Thr2: %d\n", tmp.thread1, tmp.thread2);
        printf("\tlock: %p\n-------------\n", tmp.lock);
    }
    printf("\n--------------------------\n");
    printf("Total Lock Races: %d\n", totalraces);



}

bool operator==(RaceEvent a, RaceEvent b) {
    bool thr = (a.thread1 == b.thread1) && (a.thread2 == b.thread2);
    thr = thr || ((a.thread1 == b.thread2) && (b.thread1 == a.thread2));
    if (!thr) {
        return false;
    }

    bool id = (a.iid1 == b.iid1) && (a.iid2 == b.iid2);
    id = id || ((a.iid1 == b.iid2) && (b.iid2 = a.iid1));
    if (!id) {
        return false;
    }
    return true;
}


bool operator==(LockRaceEvent a, LockRaceEvent b) {
    bool thr = (a.thread1 == b.thread1) && (a.thread2 == b.thread2);
    thr = thr || ((a.thread1 == b.thread2) && (b.thread1 == a.thread2));
    if (!thr) {
        return false;
    }

    if (a.lock == b.lock) {
        return true;
    }
    return false;
}

bool Iracer::isLockRace(vectorClock myself, vectorClock access) {
    vectorClock::iterator itr;
    bool ret_val = false;
    for (itr = myself.begin(); itr != myself.end(); itr++) {
        int mine = itr->second;
        int other = access[itr->first];
        if (mine < other) {
            ret_val = true;
        }
    }
    return ret_val;
}


