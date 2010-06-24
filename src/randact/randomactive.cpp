#define UNLOCKASSERT
#include "randomactive.h"
#include <sys/time.h>

RandomActiveTester::RandomActiveTester(thrID myself) : RandomTracker(myself) {
    targetfile = "thrille-randomactive";
    raceFound = false;
    printf("NOTE: Random Active now defaults to scheduling at NO");
    printf(" memory access points, if no data races are found in");
    printf(" %s\n", targetfile.c_str());

    // 1/x of scheduling a preemption at a given scheduling point
    chanceOfPreempt = 1;

    // (sparsifyNum/sparsifyDenom) is the ratio of memory address that you
    // want to remain after call to sparsify.
    sparsifyNum = 1;
    sparsifyDenom = 1;
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
    if (log->getRandomNumber() % 200 == 1) {
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

thrID RandomActiveTester::pickNextSchedulingChoice(SchedPointInfo * s) {
    livelockPrevention();
    if (enable_map[s->thread]) {
        int preempt = log->getRandomNumber() % chanceOfPreempt;
        if (preempt == 1) {
            safe_assert(enable_map[s->thread]);
            return s->thread;
        }
    }

    thrID mychoice = -1;


    //prefer a preemption if it is possible for greater coverage
    if (enable_map[s->thread]) {
        enable_map[s->thread] = false;
        mychoice = chooseRandomThread(enable_map.begin(), enable_map.end());
        enable_map[s->thread] = true;
        if (mychoice == -1 || log->getRandomNumber() % 6 == 0) {
            mychoice = s->thread;
        }
    } else {
        mychoice = chooseRandomThread(enable_map.begin(), enable_map.end());
    }


    if (mychoice == -1) {
        bool success = false;
        while (! success) {
            mychoice = chooseRandomThread(active_testing_paused.begin(), 
                    active_testing_paused.end());
            if (mychoice == -1) {
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

void RandomActiveTester::setMemoryAsSchedulingPoint(char * mem) {
    void * tmp = log->getAddress(mem);
    log->addAddressToSchedule(tmp);
}


void RandomActiveTester::rotateInputFile(FILE * file_in, 
        char * read_token,
        char * rotated,
        int size) {
    safe_assert(file_in != NULL);
    char in[MAX_READ_SIZE];
    int written = 0;
    strncpy(rotated, "", MAX_READ_SIZE);

    while (fgets(in, MAX_READ_SIZE, file_in)) {
        written += strlen(in);
        safe_assert(written < size);
        strncat(rotated, in, MAX_READ_SIZE);
        
        fgets(in, MAX_READ_SIZE, file_in);
        written += strlen(in);
        safe_assert(written < size);
        setMemoryAsSchedulingPoint(in);
        strncat(rotated, in, MAX_READ_SIZE);
        
        fgets(in, MAX_READ_SIZE, file_in);
        written += strlen(in);
        safe_assert(written < size);
        setMemoryAsSchedulingPoint(in);
        strncat(rotated, in, MAX_READ_SIZE);
    }
    strncat(rotated, read_token, 3*MAX_READ_SIZE);
    written += strlen(read_token);
    safe_assert(written < size);

    log->scheduleAtNoMemory();
    log->sparsifyMemorySchedulingPoints(sparsifyNum, sparsifyDenom);
}

void RandomActiveTester::setTargets(FILE * target_in, char * result, int size) {
    safe_assert(target_in != NULL);
    char string_in[MAX_READ_SIZE];
    
    fgets(string_in, MAX_READ_SIZE, target_in);
    bool isLock = strncmp(string_in, "LOCK", 4) == 0;
    bool isData = strncmp(string_in, "DATA", 4) == 0;
    safe_assert(isLock || isData);
    strncpy(result, string_in, size);

    fgets(string_in, MAX_READ_SIZE, target_in);
    target1 = log->getAddress(string_in);
    strncat(result, string_in, size);

    fgets(string_in, MAX_READ_SIZE, target_in);
    target2 = log->getAddress(string_in);
    strncat(result, string_in, size);
}

void RandomActiveTester::rewriteTargetFile(FILE * target, char * output) {
    fprintf(target, "%s", output);
}

void RandomActiveTester::setTestingTargets() {
    char read_string[MAX_READ_SIZE];
    FILE * file_in = fopen(targetfile.c_str(), "r");
    setTargets(file_in, read_string, MAX_READ_SIZE);
    char out_string[8192];
    rotateInputFile(file_in, read_string, out_string, 8192);
    log->addAddressToSchedule(target1);
    log->addAddressToSchedule(target2);
    fclose(file_in);
    FILE * file_out = fopen(targetfile.c_str(), "w");
    rewriteTargetFile(file_out, out_string);
    fclose(file_out);
}
