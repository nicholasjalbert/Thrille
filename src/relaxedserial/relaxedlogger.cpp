#include "relaxedlogger.h"
        
RelaxedLogger::RelaxedLogger(string out, string in) : Logger(out, "") {
    printf("Logger: Relaxed Logger engaged\n");
    readfile = fopen(in.c_str(), "r");
    setSchedulableAddresses();
    reconstituteRelaxedSchedule();
}

RelaxedLogger::~RelaxedLogger() {
}

thrID RelaxedLogger::getNextSignalChoice() {
    if (tts_itr == threadsToSignal.end()) {
        return INVALID_THREAD;
    }
    thrID result = (*tts_itr);
    tts_itr++;
    printf("Signalling: %d\n", result);
    return result;
}

void RelaxedLogger::reconstituteRelaxedSchedule() {
    if (readfile == NULL) {
        return;
    }
    
    char tmp[MAX_READ_SIZE];
    
    while (! isReadfileEnd(tmp, MAX_READ_SIZE)) {
        string in(tmp);
        string threadToSched = in.substr(0, in.find(":"));
        thrID current;
        current = atoi(threadToSched.c_str());
        string timesToSched = in.substr(in.find(":") + 1, in.size());
        int max;
        max = atoi(timesToSched.c_str());
        
        
        fgets(tmp, MAX_READ_SIZE, readfile);
        in = string(tmp);
        string preemptAddr = in.substr(0, in.find(":"));
        void * preempt;
        char preemptAddrC[preemptAddr.length() + 1];
        strcpy(preemptAddrC, preemptAddr.c_str());
        preempt = getAddress(preemptAddrC);
        string timesToPass = in.substr(in.find(":") + 1, in.size());
        int pass;
        pass = atoi(timesToPass.c_str());
        
        RelaxedSchedStruct * schedpoint;
        schedpoint = new RelaxedSchedStruct(current, max, preempt, pass);
        
        fgets(tmp, MAX_READ_SIZE, readfile);
        int continue_loop = strncmp(tmp, "##", 2) && 
            strncmp(tmp, " ##", 3);
        int sig;
        char * continuation = tmp;
        while (continue_loop) {
            sig = strtol(continuation, &continuation, 10);
            schedpoint->signals.push_back(sig);
            continue_loop = strncmp(continuation, "##", 2);
            continue_loop = continue_loop && strncmp(continuation, " ##", 3);
        }
        relaxed_schedule.push_back(schedpoint);
    }
    /*
    vector<RelaxedSchedStruct *>::iterator itr;
    for (itr = relaxed_schedule.begin(); itr != relaxed_schedule.end(); itr++) {
        RelaxedSchedStruct * tmp = (*itr);
        tmp->print();
    }
   */ 
}

/* TODO: fix memory leak here */
RelaxedSchedStruct RelaxedLogger::getSchedulingStruct() {
    vector<RelaxedSchedStruct *>::iterator itr = relaxed_schedule.begin();
    RelaxedSchedStruct * tmp = (*itr);
    relaxed_schedule.erase(relaxed_schedule.begin());
    return *tmp;
}

void RelaxedLogger::resetThreadsToSignal(RelaxedSchedStruct sched) {
    threadsToSignal.clear();
    vector<thrID>::iterator itr;
    for (itr = sched.signals.begin(); itr != sched.signals.end(); itr++) {
        threadsToSignal.push_back((*itr));
    }
    tts_itr = threadsToSignal.begin();
}

RelaxedSchedStruct RelaxedLogger::regenerateSched() {
    if (relaxed_schedule.size() > 0) {
        RelaxedSchedStruct schedule_struct = getSchedulingStruct();
        resetThreadsToSignal(schedule_struct);
        return schedule_struct;
    } else {
        RelaxedSchedStruct tmp(INVALID_THREAD,0,NULL,0);
        return tmp;
    }
}
