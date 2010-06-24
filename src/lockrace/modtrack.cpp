#include "modtrack.h"

ModifiedRaceTracker::ModifiedRaceTracker() : HybridRaceTracker() {
    initializationHelper();
}


ModifiedRaceTracker::ModifiedRaceTracker(unsigned int wr, unsigned int rd) : 
    HybridRaceTracker(wr, rd) {
    initializationHelper();
}

ModifiedRaceTracker::~ModifiedRaceTracker() {
}

void ModifiedRaceTracker::initializationHelper() {
    printf("Using modified hybrid tracker\n");
    outfilename = "thrille-randomactive";
    eventcount = 0;
    lockprofiling = 20;
}


void ModifiedRaceTracker::beforeSignal(thrID me, pthread_cond_t * cond) {}

void ModifiedRaceTracker::afterWait(thrID me, pthread_cond_t * cond) {}


void ModifiedRaceTracker::addLockEvent(lockRaceEvent e) {
    int count = lock_profile_map[e.addr];
    if (count > lockprofiling) {
        return;
    }
    count++;
    lock_profile_map[e.addr] = count; 
    vector<lockRaceEvent> eset = lockeventset[e.addr];
    eset.push_back(e);
    lockeventset[e.addr] = eset;
}

void ModifiedRaceTracker::outputRaces() {
    printf("Races:\n");
    map<void *, vector<dataRaceEvent> >::iterator mitr;
    vector<dataRaceEvent>::iterator vitr;
    for (mitr = eventset.begin(); mitr != eventset.end(); mitr++) {
        vector<dataRaceEvent> tmpv = mitr->second;
        for (vitr = tmpv.begin(); vitr != tmpv.end(); vitr++) {
            eventcount++;
            if (eventcount % 1000 == 0) {
                printf("Events Processed: %d\n", eventcount);
            }
            checkRace((*vitr));
        }
    }

    map<void *, vector<lockRaceEvent> >::iterator mliter;
    vector<lockRaceEvent>::iterator vliter;
    for (mliter = lockeventset.begin();
            mliter != lockeventset.end(); mliter++) {
        vector<lockRaceEvent> tmplv = mliter->second;
        for (vliter = tmplv.begin(); vliter != tmplv.end(); vliter++) {
            eventcount++;
            if (eventcount % 1000 == 0) {
                printf("Events Processed: %d\n", eventcount);
            }
            checkLockRace((*vliter));
        }
    }


    ofstream * fout = new ofstream;
    fout->open(outfilename.c_str());
    dumpRaces(fout);
    fout->close();
    delete fout;

}

void ModifiedRaceTracker::checkLockRace(lockRaceEvent e) {
    vector<lockRaceEvent> addrevents = lockeventset[e.addr];
    vector<lockRaceEvent>::iterator itr;
    for (itr = addrevents.begin(); itr != addrevents.end(); itr++) {
        if (isRacing((*itr), e)) {
            lockRaceRecord tmprec((*itr).lock, e.lock);
            vector<lockRaceRecord>::iterator litr;
            bool addrace = true;
            for (litr = foundlockraces.begin(); litr != foundlockraces.end();
                    litr++) {
                if ((*litr) == tmprec) {
                    addrace = false;
                }
            }
            if (addrace) {
                foundlockraces.push_back(tmprec);
            }
        }
    }
}

void ModifiedRaceTracker::dumpRaces(ofstream * fout) {
    vector<dataRaceRecord>::iterator itr;
    if (((int) foundraces.size()) == 0 && ((int) foundlockraces.size()) == 0) {
        printf("No races discovered\n");
    }
    for (itr = foundraces.begin(); itr != foundraces.end(); itr++) {
        printf("Race: DataRaceRecord(%p, %p)\n", 
                ((dataRaceRecord)(*itr)).getLeft(),       
                ((dataRaceRecord)(*itr)).getRight());
        (*fout) << "DATA" << endl 
            << ((dataRaceRecord)(*itr)).getLeft() << endl 
            << ((dataRaceRecord) (*itr)).getRight() << endl << flush; 
    }

    vector<lockRaceRecord>::iterator litr;
    for (litr = foundlockraces.begin(); litr != foundlockraces.end(); litr++) {
        printf("Race: LockRaceRecord(%p, %p)\n", 
                ((lockRaceRecord)(*litr)).getLeft(),       
                ((lockRaceRecord)(*litr)).getRight());
        (*fout) << "LOCK" << endl 
            << ((lockRaceRecord)(*litr)).getLeft() << endl 
            << ((lockRaceRecord) (*litr)).getRight() << endl << flush; 
    }
   
    int racecount = 0; 

    
    map<void *, vector<dataRaceEvent> > potraces = eventset;
    map<void *, vector<dataRaceEvent> >::iterator mitr;
    vector<dataRaceEvent>::iterator sitr;
    vector<dataRaceEvent> tmp;
    for (mitr = potraces.begin(); mitr != potraces.end(); mitr++) {
        tmp = mitr->second; 
        for (sitr = tmp.begin(); sitr != tmp.end(); sitr++) {
            racecount++;
        }
    }
    printf("\nData Race Events: %d\n", racecount);

    int lockracecount = 0;
    map<void *, vector<lockRaceEvent> > potlockraces = lockeventset;
    map<void *, vector<lockRaceEvent> >::iterator mlitr;
    vector<lockRaceEvent>::iterator slitr;
    vector<lockRaceEvent> ltmp;
    for (mlitr = potlockraces.begin();
            mlitr != potlockraces.end(); mlitr++) {
        ltmp = mlitr->second;
        for (slitr = ltmp.begin(); slitr != ltmp.end(); slitr++) {
            lockracecount++;
        }
    }
    printf("\nLock Race Events: %d\n", lockracecount);

}


