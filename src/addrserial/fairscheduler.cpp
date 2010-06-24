#include "fairscheduler.h"

FairScheduler::FairScheduler() {

}

FairScheduler::~FairScheduler() {

}

thrID FairScheduler::nonpreemptiveFairSchedule(SchedPointInfo * s) {
    updateDataStructures(s);
    map<thrID, bool> fenmap = getFairEnabledMap(s);
    
    map<thrID, bool>::iterator itr;
    for (itr = fenmap.begin(); itr != fenmap.end(); itr++) {
        s->fenmap[itr->first] = itr->second;
    }

    if (fenmap[s->thread]) {
        return s->thread;
    } else {
        for (itr = fenmap.begin(); itr != fenmap.end(); itr++) {
            if (itr->second) {
                return itr->first;
            }
        }
    }
    return -1;
}


bool FairScheduler::isFairEnabled(thrID target, map<thrID, bool> enmap) {
    if (! enmap[target]) {
        return false;
    }

    map<int, set<int> >::iterator mitr;
    thrID thread;
    for (mitr = fair_priority.begin(); mitr != fair_priority.end(); mitr++) {
        thread = mitr->first;
        if (enmap[thread]) {
            if (mitr->second.find(target) != mitr->second.end()) {
                return false;
            } 
        }
    }
    return true;
}

map<thrID, bool> FairScheduler::getFairEnabledMap(SchedPointInfo * s) {
    map<thrID, bool> enable_map = s->enabled;
    map<thrID, bool> return_map;

    map<thrID, bool>::iterator itr;
    thrID thread;
    for (itr = enable_map.begin(); itr != enable_map.end(); itr++) {
        thread = itr->first;
        return_map[thread] = isFairEnabled(thread, enable_map);
    }

    prev_ES.clear();

    for (itr = enable_map.begin(); itr != enable_map.end(); itr++) {
        thread = itr->first;
        if (enable_map[thread]) {
            prev_ES.insert(thread);
        }
    }

    return return_map;
}

void FairScheduler::updateDataStructures(SchedPointInfo * s) {
    thrID last_scheduled = s->thread; 
    map<thrID, bool> enable_map = s->enabled;
    bool justYield = s->isYield;

    //line 13
    fair_priority.erase(last_scheduled);

    map<thrID, bool>::iterator mitr;
    set<thrID>::iterator sitr;
    thrID thread;
    thrID tmp_thr;
    //Loop over all threads
    for (mitr = enable_map.begin(); mitr != enable_map.end(); mitr++) {
        thread = mitr->first;
        //line 15 (set intersection)
        set<thrID> & en = fair_enabled[thread];
        for (sitr = en.begin(); sitr != en.end(); ) {
            tmp_thr = (*sitr);
            if (! enable_map[tmp_thr]) {
                en.erase(sitr++);
            } else {
                ++sitr;
            }
        } 
        // line 16
        if (thread == last_scheduled) {
            for (sitr = prev_ES.begin(); sitr != prev_ES.end(); sitr++) {
                tmp_thr = (*sitr);
                if (! enable_map[tmp_thr]) {
                    fair_disabled[thread].insert(tmp_thr);
                } 
            }
        }

        //line 21 
        fair_scheduled[thread].insert(last_scheduled);
    }
    if (justYield) {

        set<thrID> H;
        set<thrID> & en = fair_enabled[last_scheduled];
        set<thrID> & dis = fair_disabled[last_scheduled];
        set<thrID> & sch = fair_scheduled[last_scheduled];
        //line 24 (calculate H)
        for (sitr = en.begin(); sitr != en.end(); sitr++) {
            tmp_thr = (*sitr);
            if (sch.find(tmp_thr)== sch.end()) {
                H.insert(tmp_thr);
            }
        }
        for (sitr = dis.begin(); sitr != dis.end(); sitr++) {
            tmp_thr = (*sitr);
            if (sch.find(tmp_thr) == sch.end()) {
                H.insert(tmp_thr);
            }
        }

        
        for (sitr = H.begin(); sitr != H.end(); sitr++) {
            tmp_thr = (*sitr);
            fair_priority[tmp_thr].insert(last_scheduled);
        }
        en.clear(); 
        for (mitr = enable_map.begin(); mitr != enable_map.end(); mitr++) {
            tmp_thr = mitr->first;
            if (enable_map[tmp_thr]) {
                en.insert(tmp_thr);
            }
        }
        fair_disabled[last_scheduled].clear();
        fair_scheduled[last_scheduled].clear();
    } 
}



