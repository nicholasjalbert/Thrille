#include "logger.h"

void Logger::assertFail(string file, string cond, int line) {
    if (print) {
        printf("LOG: assertion failure in file %s at line %d\n",
                file.c_str(), line); 
        fflush(stdout);
    }
    if (schedulefile != NULL) {
        size_t loc = file.rfind("/");
        if (loc != string::npos) {
            file = file.substr(loc + 1);
        }
        printCurrentScheduleToFile(schedulefile);
        fprintf(schedulefile, "thrille assert fail %s ", file.c_str());
        fprintf(schedulefile, "%s\n", cond.c_str());
        fflush(schedulefile);
    }
}

void Logger::programAssertFail(thrID myself, void * ret_addr) {
    printf("Thrille: program assertion failure on thread %d at id %p\n",
            myself, ret_addr);
    fflush(stdout);
    if (schedulefile != NULL) {
        printCurrentScheduleToFile(schedulefile);
        fprintf(schedulefile, "program assert fail [%d]", myself);
        fprintf(schedulefile, " %p\n", ret_addr);
        fflush(schedulefile);
    }
}

void Logger::programDeadlock() {
    printf("Thrille: program deadlock\n");
    fflush(stdout);
    if (schedulefile != NULL) {
        printCurrentScheduleToFile(schedulefile);
        fprintf(schedulefile, "program deadlock\n"); 
        fflush(schedulefile);
    }
}

void Logger::segfault(thrID myself) {
    printf("SEGFAULT by thread %d\n", myself);
    fflush(stdout);
    if (schedulefile != NULL) {
        printCurrentScheduleToFile(schedulefile);
        fprintf(schedulefile, "segfault thread %d\n", myself);
        fflush(schedulefile);
    }

}

void Logger::setPrint() { 
    string thrille (getenv("THRILLE_ROOT"));
    thrille += "/src/serializer/config/thrille-print";
    char in[MAX_READ_SIZE];
    FILE * file = NULL;
    file = fopen(thrille.c_str(), "r");

    if (file == NULL) {
        printf("Logger: error no print configuration file\n");
        print = false;
        return;
    }

    fgets(in, MAX_READ_SIZE, file);
    print = static_cast<bool>(atoi(in));
    fclose(file);
}


// sdbm hash function for strings
unsigned long Logger::getLogHash() {
    unsigned long hash = 0;
    FILE * hash_sched;
    char s[MAX_READ_SIZE]; 

    hash_sched = fopen(_infile.c_str(), "r");
    
    if (hash_sched == NULL) {
        return 0;
    }

    while (fgets(s, MAX_READ_SIZE, hash_sched)) {
        int i = 0;
        while (s[i] != '\0') {
            hash = s[i] + (hash << 6) + (hash << 16) - hash;
            i++;
        }
    }
    fclose(hash_sched);
    return hash;
}

void Logger::seedGenerator(unsigned long l) {
    myrand->seed(l);
}


// returns random number [0,upper]
int Logger::getRandomNumber(int upper) {
    return myrand->randInt(upper);
}

// returns random number [0,2^32 - 1]
int Logger::getRandomNumber() {
    return myrand->randInt();
}


int Logger::getBadRandomNumber() {
    return rand();
}

void Logger::allocateObjects() {
    myrand = new MTRand();
    myrand->seed();
    scheduleAll = false;
    schedulefile = NULL;
    readfile = NULL;
}


bool Logger::getPrint() {
    return print;
}

void * Logger::getAddress(char * in) {
    unsigned long l = strtoul(in, NULL, 16);
    return (void *) l;
}

void Logger::setSchedulableAddresses() {
    if (readfile == NULL) {
        return;
    }
    char in[MAX_READ_SIZE];
    
    fgets(in, MAX_READ_SIZE, readfile);
    safe_assert(strncmp(in, "begin_addr_list", 15) == 0);

    while (fgets(in, MAX_READ_SIZE, readfile)) {
        if (strncmp(in, "end_addr_list", 13) == 0) {
            break;
        }
        if (strncmp(in, "all", 3) == 0) {
            fgets(in, MAX_READ_SIZE, readfile);
            scheduleAll = true;
            printf("Logger: Scheduling at all memory accesses\n");
            break;
        }
        void * address = getAddress(in);
        addAddressToSchedule(address);
    }
    
    safe_assert(strncmp(in, "end_addr_list", 13) == 0); 
}

void Logger::reconstituteSchedule() {
    if (readfile == NULL) {
        return;
    }
    char in[MAX_READ_SIZE];

    while(! isReadfileEnd(in, MAX_READ_SIZE)) {
        if (strncmp(in, "SCHED", 5) == 0) {
            thrID chosen = parseNextReadfileLineAsThrID();
            thrID caller = parseNextReadfileLineAsThrID();
            string desc = parseNextReadfileLineAsString();
            void* idaddr = parseNextReadfileLineAsVoidStar();
            void* mem_1 = parseNextReadfileLineAsVoidStar();
            void* mem_2 = parseNextReadfileLineAsVoidStar();
            clearLinesFromReadfile(1);

            ScheduleDecisionInfo * info;
            info = new ScheduleDecisionInfo(chosen, caller, desc,
                    idaddr, mem_1, mem_2);
            read_schedule.push_back(info);
        } else {
            safe_assert(strncmp(in, "SIGNAL", 6) == 0);
            thrID chosen = parseNextReadfileLineAsThrID();
            thrID caller = parseNextReadfileLineAsThrID();
            void * idaddr = parseNextReadfileLineAsVoidStar();
            void * cond = parseNextReadfileLineAsVoidStar();
            clearLinesFromReadfile(1);
            
            SignalDecisionInfo * info; 
            info = new SignalDecisionInfo(chosen, caller, idaddr, cond);
            read_schedule.push_back(info);
        }
    }

    fclose(readfile);
    readfile = NULL;
}



Logger::Logger(string outfile, string infile) {
    _infile = infile;
    _outfile = outfile;
    setPrint();
    allocateObjects();
    
    schedulefile = fopen(outfile.c_str(), "w");
    safe_assert(schedulefile != NULL);
    
    readfile = fopen(infile.c_str(), "r");
    setSchedulableAddresses();
    reconstituteSchedule();

    schedulePointsToSkip = 0;
    pointsLeftToSkip = schedulePointsToSkip;
    printf("Schedule Hash ID: %lu\n", getLogHash());
}

void Logger::deallocateSchedule() {
    vector<DecisionInfo *>::iterator itr;
    for (itr = schedule.begin(); itr != schedule.end(); itr++) {
        DecisionInfo * tmp = (*itr);
        delete tmp;
    }
}
void Logger::deallocateReadSchedule() {
    vector<DecisionInfo *>::iterator itr;
    for (itr = read_schedule.begin(); itr != read_schedule.end(); itr++) {
        DecisionInfo * tmp = (*itr);
        delete tmp;
    }
}

Logger::~Logger() {
    printCurrentScheduleToFile(schedulefile);
    cleanClose();
    deallocateSchedule();
    deallocateReadSchedule();
    if (readfile != NULL) {
        fclose(readfile);
        readfile = NULL;
    }
    if (schedulefile != NULL) {
        fclose(schedulefile);
        schedulefile = NULL;
    }
    delete myrand;
}

void Logger::scheduleAtAllMemory() {
    scheduleAll = true;
}

/* Keeps a random [num/denom] memory points to schedule at */
void Logger::sparsifyMemorySchedulingPoints(int num, int denom) {
    if (num == denom) {
        return;
    }
    set<void *>::iterator itr;
    itr = addressToSchedule.begin();
    while (itr != addressToSchedule.end()) {
        bool keepElement = false;
        int i = 0;
        while (!keepElement && i < num) {
            keepElement = (getRandomNumber() % denom) ? false : true;
            i += 1;
        }

        if (!keepElement) {
            addressToSchedule.erase(itr++);
        } else {
            ++itr; 
        }
    }
}

void Logger::scheduleAtNoMemory() {
    scheduleAll = false;
}


void Logger::recordAddrSchedulingPoints() {
    fprintf(schedulefile, "begin_addr_list\n");
    if (scheduleAll) {
            fprintf(schedulefile, "all\n");
    } else {
        set<void *>::iterator itr;
        for (itr = addressToSchedule.begin();
                itr != addressToSchedule.end(); itr++) {
            fprintf(schedulefile, "%p\n", (*itr));
        }
    }
    fprintf(schedulefile, "end_addr_list\n");
}

void Logger::flushLogs() {
    fflush(schedulefile);
}

void Logger::BeforeMutexTrylock(thrID target) {
    if (print) {
        printf("LOG: thread %d is before a trylock\n", target);
        fflush(stdout);
    }
}

void Logger::SimulateMutexTrylock(thrID target) {
    if (print) {
        printf("LOG: thread %d is about to simulate a trylock\n", target);
        fflush(stdout);
    }
}


int Logger::numAddrToSchedule() {
    return ((int) addressToSchedule.size());
}

void Logger::addAddressToSchedule(void * addr) {
    addressToSchedule.insert(addr);
}

bool Logger::isSchedulingPoint(void * addr) {
    if (scheduleAll) {
        return true;
    }
    if (addressToSchedule.find(addr) != addressToSchedule.end()) {
        return true;
    }
    return false;
}

bool Logger::tryToSchedule() {
    if (pointsLeftToSkip == 0) {
        pointsLeftToSkip = schedulePointsToSkip;
        return true;
    }
    pointsLeftToSkip--;
    return false;
}

void Logger::cleanClose() {
    if (schedulefile != NULL) {
        fprintf(schedulefile, "end_log\n");
    }
    printf("Closing log\n");
}

/* 
 * returns true if file end false otherwise, 
 * places the token into read_result 
 */
bool Logger::isReadfileEnd(char * read_result, int size) {
    char * result = fgets(read_result, size, readfile);
    
    int isEndLog = (strncmp(read_result, "end_log", 7) == 0);
    int isSafeAssert = (strncmp(read_result, "thrille assert", 14) == 0); 
    int isDeadlock = (strncmp(read_result, "program deadlock", 16) == 0);
    int isSegfault = (strncmp(read_result, "segfault", 8) == 0);
    int isProgAssert = (strncmp(read_result, "program assert", 14) == 0);

    if (result == NULL || isEndLog || isSafeAssert || isDeadlock ||
            isSegfault || isProgAssert) {
        return true;
    }
    return false;
}

void Logger::checkToken(char * in, char * cmp, int size) {
    int is_expected = strncmp(in, cmp, size);
    safe_assert(is_expected == 0);
}

thrID Logger::parseNextReadfileLineAsThrID() {
    char in[MAX_READ_SIZE];
    fgets(in, MAX_READ_SIZE, readfile);
    setWhitespaceToColon(in, MAX_READ_SIZE);
    return atoi(in);
}

void Logger::clearLinesFromReadfile(int lines) {
    char in[MAX_READ_SIZE];
    int i = 0;
    while (i < lines) {
        fgets(in, MAX_READ_SIZE, readfile);
        i++;
    }
}


thrID Logger::getNextChoice() {
    vector<DecisionInfo *>::iterator itr;
    itr = read_schedule.begin();
    DecisionInfo * d = (*itr);
    thrID nextchoice = d->getChosen();
    
    char tmp_buffer[MAX_READ_SIZE];
    sprintf(tmp_buffer, "%p", d->getID());
    lastID = string(tmp_buffer);

    delete d;
    read_schedule.erase(itr);
    return nextchoice;
}

void Logger::closeReadfile() {
    if (readfile == NULL) {
        return;
    }
    fclose(readfile);
    readfile = NULL;
    if (print) {
        printf("LOG: no longer following previous schedule...\n");
    }
}

char * Logger::trimWhitespace(char * str) {
    char * end;
    while (isspace(*str)) {
        str++;
    }
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) {
        end--;
    }
    *(end+1) = 0;
    return str;
}

void Logger::setWhitespaceToColon(char * in, int size) {
    int i = 0;
    while (i < size) {
        if (in[i] == ':') {
            in[i] = ' ';
            break;
        }
        in[i] = ' ';
        i++;
    }
}


void * Logger::parseNextReadfileLineAsVoidStar() {
    char * s = parseNextReadfileLineAsCharStar();
    return getAddress(s);
}

string Logger::parseNextReadfileLineAsString() {
    char * s = parseNextReadfileLineAsCharStar();
    return string(s);
}

char * Logger::parseNextReadfileLineAsCharStar() {
    char in[MAX_READ_SIZE];
    char * result;
    fgets(in, MAX_READ_SIZE, readfile);
    setWhitespaceToColon(in, MAX_READ_SIZE);
    result = trimWhitespace(in);
    return result;
}

thrID Logger::getNextSignalChoice() {
    if (read_schedule.size() <= 0) {
        return INVALID_THREAD;
    }
    return getNextChoice();
}

thrID Logger::getNextScheduleChoice(SchedPointInfo * s) {
    if (read_schedule.size() <= 0) {
        return INVALID_THREAD;
    }
    return getNextChoice();
}

string Logger::getLastID() {
    return lastID;
}

/* modifies s so that Logger::scheduling can be used to record
 * thread info when fair scheduling is enabled
 */
void Logger::fairScheduling(thrID target, SchedPointInfo *s) {
    map<thrID, bool> save_map;
    map<thrID, bool>::iterator itr;

    for (itr = s->enabled.begin(); itr != s->enabled.end(); itr++) {
        save_map[itr->first] = itr->second;
    }

    s->enabled.clear();

    for (itr = s->fenmap.begin(); itr != s->fenmap.end(); itr++) {
        s->enabled[itr->first] = itr->second;
    }

    scheduling(target, s);

    s->enabled.clear();

    for (itr = save_map.begin(); itr != save_map.end(); itr++) {
        s->enabled[itr->first] = itr->second;
    }
}



void Logger::scheduling(thrID target, SchedPointInfo * s) {
    if (print) {
        printf("LOG: thread %d chosen to execute\n", target);
        printf("\tThread calling sched: %d\n", s->thread);
        printf("\tScheduling at point: %s\n", s->type.c_str());
        printf("\tSched point addr: %p\n", s->ret_addr);
        printf("\tEnabled:\n");
        map<thrID, bool>::iterator itr;
        for (itr = s->enabled.begin(); itr != s->enabled.end(); itr++) {
            if (itr->second) {
                printf("\t\tThread %d: enabled\n", itr->first);
            } else {
                printf("\t\tThread %d: disabled\n", itr->first);
            }
        }
        fflush(stdout);
    }

    thrID chosen = target;
    thrID caller = s->thread;
    string desc = s->type;
    void* id = s->ret_addr;
    void* mem_1 = s->memory_accessed_1;
    void* mem_2 = s->memory_accessed_2;

    ScheduleDecisionInfo * info; 
    info = new ScheduleDecisionInfo(chosen, caller, desc, id, mem_1, mem_2);
    map<thrID, bool>::iterator itr;
    for (itr = s->enabled.begin(); itr != s->enabled.end(); itr++) {
        if (itr->second) {
            info->addEnabledThread(itr->first);
        }
    }
    schedule.push_back(info);
}

void Logger::ThreadStart(thrID myself, void * arg) {
    if (print) {
        printf("LOG: thread %d started\n", myself);
        fflush(stdout);
    }
}

void Logger::ThreadFinish(thrID myself, void * arg) {
    if (print) {
        printf("LOG: thread %d finished\n", myself);
        fflush(stdout);
    }
}


void Logger::BeforeCreate(thrID myself, thrID target) {
    if (print) {
        printf("LOG: thread %d is about to create thread %d\n", myself, target);
        fflush(stdout);
    }
}

void Logger::AfterCreate(thrID myself) {
    if (print) {
        printf("LOG: thread %d just created a thread\n", myself);
        fflush(stdout);
    }
}

void Logger::pauseThread(thrID myself) {
    if (print) {
        printf("LOG: thread %d is about to pause\n", myself);
        fflush(stdout);
    }
}

void Logger::BeforeSchedPoint(thrID myself) {
    if (print) {
        printf("LOG: thread %d is before generic schedule point\n", myself);
        fflush(stdout);
    }
}

void Logger::BeforeJoin(thrID myself, thrID target) {
    if (print) {
        printf("LOG: thread %d is about to join thread %d\n", myself, target);
        fflush(stdout);
    }
}

void Logger::BeforeMutexLock(thrID myself,
        void * addr, pthread_mutex_t * l, bool isWait) {
    if (print && isWait) {
        printf("LOG: thread %d is IN A WAIT before lock id %p (addr %p)\n",
                myself, addr, l);
        fflush(stdout);
    } else if (print && ! isWait) {
        printf("LOG: thread %d is before lock id %p (addr %p)\n",
                myself, addr, l);
        fflush(stdout);
    }

}

void Logger::SimulateMutexLock(thrID myself, bool isWait) {
    if (print && isWait) {
        printf("LOG: thread %d is IN A WAIT simulating a lock\n", myself);
        fflush(stdout);
    } else if (print && ! isWait) {
        printf("LOG: thread %d is simulating a lock\n", myself);
        fflush(stdout);
    }

}

void Logger::AfterMutexUnlock(thrID myself, bool isWait) {
    if (print && isWait) {
        printf("LOG: thread %d IN WAIT after an unlock\n", myself);
        fflush(stdout);
    } else if (print && ! isWait) {
        printf("LOG: thread %d after an unlock\n", myself);
        fflush(stdout);
    }
}

void Logger::waitPauseThread(thrID target) {
    if (print) {
        printf("LOG: thread %d about to pause at a wait\n", target);
        fflush(stdout);
    }
}

void Logger::waitWakeThread(thrID target) {
    if (print) {
        printf("LOG: thread %d about to be awoken from a wait\n", target);
        fflush(stdout);
    }
}

void Logger::SimulateCondWait(thrID myself) {
    if (print) {
        printf("LOG: thread %d is simulating a wait\n", myself);
        fflush(stdout);
    }
}

void Logger::BeforeCondWait(thrID myself) {
    if (print) {
        printf("LOG: thread %d is before a wait\n", myself);
        fflush(stdout);
    }
}

void Logger::myMemoryRead(thrID myself, void * addr) {
    if (print) {
        printf("LOG: thread %d is at a memory read %p\n", myself, addr);
        fflush(stdout);
    }
}

void Logger::myMemoryWrite(thrID myself, void * addr) {
    if (print) {
        printf("LOG: thread %d is at a memory write %p\n", myself, addr);
        fflush(stdout);
    }
}

void Logger::signalScheduling(thrID target, SignalPointInfo * s) { 
    if (print) {
        if (s->isBroadcast) {
            printf("LOG: thread %d is BROADCASTING to thread %d\n",
                    s->thread, target);
        } else {
            printf("LOG: thread %d is signalling thread %d\n",
                    s->thread, target);
        }
        printf("\tCond variable:%p\n", s->cond);
        printf("\tAddr of signal:%p\n", s->ret_addr);
        map<thrID, pthread_cond_t *>::iterator itr;
        for (itr = s->cond_map.begin();
                itr != s->cond_map.end(); itr++) {
            if (itr->second == s->cond) {
                printf("\t\t%d: on cond\n", itr->first);
            } else {
                printf("\t\t%d: not\n", itr->first);
            }
        }
        fflush(stdout);
    }
    
    //Log info if not a broadcast
    if (! s->isBroadcast) {

        thrID chosen = target;
        thrID caller = s->thread;
        void * id = s->ret_addr;
        void * cond = s->cond;

        SignalDecisionInfo * info; 
        info = new SignalDecisionInfo(chosen, caller, id, cond);


        map<thrID, pthread_cond_t *>::iterator itr;
        for (itr = s->cond_map.begin(); itr != s->cond_map.end(); itr++) {
            if (itr->second == s->cond) {
                info->addThreadOnCond(itr->first);
            }
        }

        schedule.push_back(info);
    }
}

void Logger::printCurrentSchedule() {
    printCurrentScheduleToFile(stdout);
}

void Logger::printCurrentScheduleToFile(FILE * file) {
    recordAddrSchedulingPoints();
    vector<DecisionInfo *>::iterator itr;
    for (itr = schedule.begin(); itr != schedule.end(); itr++) {
        DecisionInfo * tmp = (*itr);
        tmp->printToFile(file);
    }
}



void Logger::SignalMissed(thrID myself, bool isbroad) {
    if (isbroad) {
        if (print) {
            printf("LOG: thread %d sent a missed broadcast\n", myself);
            fflush(stdout);
        }
    } else {
        if (print) {
            printf("LOG: thread %d sent a missed signal\n", myself);
            fflush(stdout);
        }
    }
}

void Logger::BeforeCondTimedwait(thrID myself) {
    if (print) {
        printf("LOG: thread %d is before a timed_wait\n", myself);
        fflush(stdout);
    }
}

void Logger::SimulateCondTimedwait(thrID myself) {
    if (print) {
        printf("LOG: thread %d is simulating a timed_wait\n", myself);
        fflush(stdout);
    }
}

void Logger::BeforeExit(thrID myself) {
    if (print) {
        printf("LOG: thread %d is before a pthread_exit\n", myself);
        fflush(stdout);
    }
}

void Logger::SimulateExit(thrID myself) {
    if (print) {
        printf("LOG: thread %d is simulating a pthread_exit\n", myself);
        fflush(stdout);
    }
}

void Logger::myUsleep(thrID myself) {
    if (print) {
        printf("LOG: thread %d is usleeping...\n", myself);
        fflush(stdout);
    }
}

void Logger::mySleep(thrID myself) {
    if (print) {
        printf("LOG: thread %d is sleeping...\n", myself);
        fflush(stdout);
    }
}

void Logger::timedwaitNoTimeout(thrID myself) {
    if (print) {
        printf("LOG: thread %d was signalled on timedwait...\n", myself);
        fflush(stdout);
    } 
}

void Logger::timedwaitTimeout(thrID myself) {
    if (print) {
        printf("LOG: thread %d timedout on timedwait...\n", myself);
        fflush(stdout);
    }
}

void Logger::BeforeSemWait(thrID myself) {
    if (print) {
        printf("LOG: thread %d before sem wait...\n", myself);
        fflush(stdout);
    }
}

void Logger::SimulateSemWait(thrID myself) {
    if (print) {
        printf("LOG: thread %d is simulate sem wait...\n", myself);
        fflush(stdout);
    }
}

void Logger::SimulateSemPost(thrID myself) {
    if (print) {
        printf("LOG: thread %d simulating sem post...\n", myself);
        fflush(stdout);
    }
}

void Logger::BeforeBarrierInit(thrID myself) {
    if (print) {
        printf("LOG: thread %d before barrier init...\n", myself);
        fflush(stdout);
    }

}

void Logger::BeforeBarrierWait(thrID myself) {
    if (print) {
        printf("LOG: thread %d before barrier wait...\n", myself);
        fflush(stdout);
    }

}

