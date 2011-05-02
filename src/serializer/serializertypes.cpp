#include "serializertypes.h"

DecisionInfo::DecisionInfo(thrID _chosen, thrID _caller, void * _id) {
    chosen = _chosen;
    caller = _caller;
    idaddr = _id;
}


DecisionInfo::~DecisionInfo() { }

void DecisionInfo::printToStdout() {
    printToFile(stdout);
}

thrID DecisionInfo::getChosen() {
    return chosen;
}

void * DecisionInfo::getID() {
    return idaddr;
}

ScheduleDecisionInfo::ScheduleDecisionInfo(thrID _chosen, thrID _caller,
        string _desc, void* _id, void* mem_1 = NULL, void* mem_2 = NULL) : 
    DecisionInfo(_chosen, _caller, _id) {
    description = changeSpacesToUnderscores(_desc);
    memory_accessed_1 = mem_1;
    memory_accessed_2 = mem_2;
}

string ScheduleDecisionInfo::changeSpacesToUnderscores(string my_str) {
    string::iterator sitr;
    for (sitr = my_str.begin(); sitr != my_str.end(); sitr++) {
        if (((char)(*sitr)) == ' ') {
            my_str.replace(sitr, sitr + 1, "_");
        }
    }
    return my_str;
}

void ScheduleDecisionInfo::printToFile(FILE * file) {
    fprintf(file, "SCHED\n");
    fprintf(file, "chosen:%d\n", chosen);
    fprintf(file, "caller:%d\n", caller);
    fprintf(file, "typstr:%s\n", description.c_str());
    fprintf(file, "idaddr:%p\n", idaddr);
    if (memory_accessed_1 != NULL) {
        fprintf(file, "memor1:%p\n", memory_accessed_1);
    } else {
        fprintf(file, "memor1:0x0\n");
    }
    if (memory_accessed_2 != NULL) {
        fprintf(file, "memor2:%p\n", memory_accessed_2);
    } else {
        fprintf(file, "memor2:0x0\n");
    }
    fprintf(file, "enable:");
    vector<thrID>::iterator itr;
    for (itr = enabled.begin(); itr != enabled.end(); itr++) {
        fprintf(file, "%d,", (*itr));
    }
    fprintf(file, "\n");
    fflush(file);
}

void ScheduleDecisionInfo::addEnabledThread(thrID _thread) {
    enabled.push_back(_thread);
}


SignalDecisionInfo::SignalDecisionInfo(thrID _chosen, thrID _caller,
        void * _id, void * _cond) : DecisionInfo(_chosen, _caller, _id) {
    cond = _cond;
} 

void SignalDecisionInfo::printToFile(FILE * file) {
    fprintf(file, "SIGNAL\n");
    fprintf(file, "signalled:%d\n", chosen);
    fprintf(file, "caller:%d\n", caller);
    fprintf(file, "idaddr:%p\n", idaddr);
    fprintf(file, "cond:%p\n", cond);

    fprintf(file, "oncond:");
    vector<thrID>::iterator itr;
    for (itr = oncond.begin(); itr != oncond.end(); itr++) {
        fprintf(file, "%d,", (*itr));
    }
    fprintf(file, "\n");
    fflush(file);
} 

void SignalDecisionInfo::addThreadOnCond(thrID _thread) {
    oncond.push_back(_thread);

} 





