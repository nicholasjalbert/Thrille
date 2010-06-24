#define UNLOCKASSERT
#include "randomdata.h"

RandomDataTester::RandomDataTester(thrID myself) : RandomActiveTester(myself) {
    setTestingTargets();
    printf("Random Active (Data) Testing started...\n");
    printf("iid 1: %p, iid 2: %p\n", target1, target2);
}

RandomDataTester::RandomDataTester(thrID myself,
        bool testing) : RandomActiveTester(myself) {
    printf("TEST: Random Active (Data) Testing started...\n");
    target1 = 0;
    target2 = 0;
}

RandomDataTester::~RandomDataTester() {
    if (raceFound) {
        printf("Data Race Between %p and %p Found!\n", 
                target1, target2);
    } else {
        printf("Data Race Between %p and %p Not Reproduced\n", 
                target1, target2);
    }
}

bool RandomDataTester::reenableThreadIfLegal(thrID thr) {
    safe_assert(active_testing_paused[thr]);
    active_testing_paused[thr] = false;
    enableThread(thr);
    return true;

}

void RandomDataTester::handleMyMemoryRead(thrID myself, 
        void * iid, void * addr) {
    if (iid == target1 || iid == target2) {
        ActiveRaceInfo tmp(myself, addr, false);
        vector<thrID> racers = isRacing(tmp);
        if (((int) racers.size()) > 0) {
            raceFound = true;
            if (log->getPrint()) {
                printf("Racing access discovered\n");
            }
            enableSpecificActiveTestingPaused(racers);
        } else {
            active_testing_paused[myself] = true;
            active_testing_info[myself] = tmp;
            disableThread(myself);
        }
    }
}

void RandomDataTester::handleMyMemoryWrite(thrID myself, 
        void * iid, void * addr) {
    if (iid == target1 || iid == target2) {
        ActiveRaceInfo tmp(myself, addr, true);
        vector<thrID> racers = isRacing(tmp);
        if (((int) racers.size()) > 0) {
            raceFound = true;
            if (log->getPrint()) {
                printf("Racing access discovered\n");
            }
            enableSpecificActiveTestingPaused(racers);
        } else {
            active_testing_paused[myself] = true;
            active_testing_info[myself] = tmp;
            disableThread(myself);
        }
    }
}

//TODO: cleanup this input stuff
void RandomDataTester::setTestingTargets() {
    ifstream * infile = new ifstream();
    infile->open(targetfile.c_str());
    safe_assert(infile->is_open());
    string type;
    (*infile) >> type;
    safe_assert(type == "DATA");
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


