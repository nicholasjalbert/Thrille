#define UNLOCKASSERT
#include "randomactive.h"

RandomActiveTester::RandomActiveTester(thrID myself) : RandomTracker(myself) {
    targetfile = "thrille-randomactive";
    raceFound = false;
}


RandomActiveTester::~RandomActiveTester() {
}


vector<thrID> RandomActiveTester::isRacing(ActiveRaceInfo potentialRace) {
    vector<thrID> allracers;
    map<thrID, bool>::iterator itr;
    for (itr = active_testing_paused.begin(); 
            itr != active_testing_paused.end(); itr++) {
        bool ispaused = itr->second;
        if (ispaused) {
            thrID thread = itr->first;
            ActiveRaceInfo info = active_testing_info[thread];
            safe_assert(info.me != -1);
            bool isRace = info.racesWith(potentialRace);
            if (isRace) {
                allracers.push_back(thread);
            }
        }
    }
    return allracers;
}

void RandomActiveTester::enableSpecificActiveTestingPaused(
        vector<thrID> targets) {
    vector<thrID>::iterator itr;
    for (itr = targets.begin(); itr != targets.end(); itr++) {
        safe_assert(active_testing_paused[(*itr)]);
        reenableThreadIfLegal((*itr));
    }

}

void RandomActiveTester::enableAllActiveTestingPaused() {
    map<thrID, bool>::iterator itr;
    for (itr = active_testing_paused.begin(); 
            itr != active_testing_paused.end(); itr++) {
        if (itr->second) {
            reenableThreadIfLegal(itr->first);
        }
    }
}


void RandomActiveTester::livelockPrevention() {
    if (rand() % 500 == 1) {
        thrID livelockprevent = 
            chooseRandomThread(active_testing_paused.begin(), 
                    active_testing_paused.end());
        if (livelockprevent != -1) {
            if (log->getPrint()) {
                printf("Livelock prevention, enabling a paused thread\n");
            }
            reenableThreadIfLegal(livelockprevent);
        }
    }
}

thrID RandomActiveTester::pickNextSchedulingChoice() {
    livelockPrevention();
    thrID mychoice = chooseRandomThread(enable_map.begin(), enable_map.end());
    if (mychoice == -1) {
        bool success = false;
        while (! success) {
            mychoice = chooseRandomThread(active_testing_paused.begin(), 
                    active_testing_paused.end());
            if (mychoice == -1) {
                printf("Real Deadlock detected\n");
                safe_assert(false);
                return -1;
            } else {
                if (log->getPrint()) {
                    printf("Enabling a postponed thread\n");
                }
                success = reenableThreadIfLegal(mychoice);
            }
        }
    }
    safe_assert(enable_map[mychoice]);
    safe_assert(active_testing_paused[mychoice] == false);
    return mychoice;
}

bool RandomActiveTester::reenableThreadIfLegal(thrID thr) {
    safe_assert(active_testing_paused[thr]);
    active_testing_paused[thr] = false;
    disableThread(thr);
    return false;
}


string RandomActiveTester::rotateInputFile(ifstream * file, 
        ostringstream * endstr) {
    safe_assert(file->is_open());
    ostringstream output;
    string m;
    (*file) >> m;
    while (m == "DATA" || m == "LOCK") {
        output << m << endl;
        (*file) >> m;
        output << m << endl;
        (*file) >> m;
        output << m << endl;
        (*file) >> m;
    }
    output << (*endstr).str();
    return output.str();
}


