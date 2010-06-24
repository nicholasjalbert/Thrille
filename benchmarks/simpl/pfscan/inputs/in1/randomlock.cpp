#define UNLOCKASSERT
#include "randomlock.h"


RandomLockTester::RandomLockTester(thrID myself) : RandomActiveTester(myself) {
    setTestingTargets();
    printf("Random Active (Lock) Testing started...\n");
    printf("Target 1: %p, Target 2: %p\n", target1, target2);
}

RandomLockTester::~RandomLockTester() {
    if (raceFound) {
        printf("Lock Race Between %p and %p Found!\n", 
                target1, target2);
    } else {
        printf("Lock Race Between %p and %p Not Reproduced\n", 
                target1, target2);
    }
}


void RandomLockTester::handleBeforeMutexLock(thrID myself,
        pthread_mutex_t * lock, void * addr, bool isWait) {
    if (addr == target1 || addr == target2) {
        ActiveRaceInfo tmp(myself, lock, true);
        vector<thrID> racers = isRacing(tmp);
        if (((int) racers.size()) > 0) {
            raceFound = true;
            if (log->getPrint()) {
                printf("Lock race discovered\n");
            }
            enableSpecificActiveTestingPaused(racers);
        } else {
            active_testing_paused[myself] = true;
            active_testing_info[myself] = tmp;
            disableThread(myself);
        }
    }
}


void RandomLockTester::handleAfterMutexUnlock(thrID, pthread_mutex_t *,
        bool isWait) {
    map<thrID, bool>::iterator itr;
    for (itr = active_testing_paused.begin();
            itr != active_testing_paused.end(); itr++) {
        if (itr->second) {
            disableThread(itr->first);
        }
    }
}

bool RandomLockTester::lockIsInvalid(pthread_mutex_t * lock) {
    int result = Originals::pthread_mutex_trylock(lock);
    if (result == EBUSY) {
        return false;
    } else if (result == 0) {
        Originals::pthread_mutex_unlock(lock);
        return false;
    } else {
        return true;
    }
}

bool RandomLockTester::reenableThreadIfLegal(thrID thr) {
    safe_assert(active_testing_paused[thr]);
    active_testing_paused[thr] = false;
    ActiveRaceInfo tmp = active_testing_info[thr];
    if (lockIsInvalid((pthread_mutex_t *) tmp.addr)) {
        printf("Lock invalid--potential error here, ");
        printf("thread %d will be unpaused (by Active Testing)\n", thr);
        enable_map[thr] = true;
    } else {
        determineLockStatus(thr, (pthread_mutex_t *) tmp.addr);
    }
    return enable_map[thr];
}

//TODO: clean this up and unify in the base class
void RandomLockTester::setTestingTargets() {
    ifstream * infile = new ifstream();
    infile->open(targetfile.c_str());
    safe_assert(infile->is_open());
    string type;
    (*infile) >> type;
    safe_assert(type == "LOCK");
    (*infile) >> target1;
    (*infile) >> target2;
    ostringstream  * str = new ostringstream();
    (*str) << type << endl << target1 
        << endl << target2 << endl;
    string output = rotateInputFile(infile, str);
    infile->close();
    ofstream * outfile = new ofstream();
    outfile->open(targetfile.c_str());
    safe_assert(outfile->is_open());
    (*outfile) << output << flush;
    outfile->close();
    delete infile;
    delete str;
    delete outfile; 

}
