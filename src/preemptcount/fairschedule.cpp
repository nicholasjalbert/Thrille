
#include "fairschedule.h"

FairScheduler::FairScheduler() {
    justYield = false;
}

FairScheduler::~FairScheduler() {

}


void FairScheduler::lastWasYield() {
    justYield = true;
}

thrID FairScheduler::getLastScheduled() {
    if (schedule_record.size() > 0) {
        return schedule_record.back();
    }
    return 0;
}

void FairScheduler::updateDataStructures(map<thrID, bool> enable_map) {
    thrID last_scheduled = getLastScheduled();

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
                en.erase(tmp_thr);
                ++sitr;
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

        //printMahMap("yield (priority)", fair_priority);
        //printMahMap("yield (enabled)", fair_enabled);
        //printMahMap("yield (disabled)", fair_disabled);
        //printMahMap("yield (scheduled)", fair_scheduled);

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
    justYield = false; 
}


map<thrID, bool> FairScheduler::getFairEnabledMap(map<thrID, bool> enable_map){

    //printThisMap("before (priority)", fair_priority);
    //printThisMap("before (enabled)", fair_enabled);
    //printThisMap("before (disabled)", fair_disabled);
    //printThisMap("before (scheduled)", fair_scheduled);

    updateDataStructures(enable_map);

    //printThisMap("after (priority)", fair_priority);
    //printThisMap("after (enabled)", fair_enabled);
    //printThisMap("after (disabled)", fair_disabled);
    //printThisMap("after (scheduled)", fair_scheduled);

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

void FairScheduler::fairChosen(thrID myself) {
    schedule_record.push_back(myself);
}

void FairScheduler::printThisSet(set<thrID> s) {
    set<thrID>::iterator itr;
    printf("{");
    for (itr = s.begin(); itr != s.end(); itr++) {
        printf("%d, ", (*itr));
    }
    printf("}");
    fflush(stdout);
}

void FairScheduler::printThisMap(string s, map<thrID, set<thrID> > m) {
    printf("%s:\n", s.c_str());
    map<int, set<int> >::iterator mitr;
    for (mitr = m.begin(); mitr != m.end(); mitr++) {
        printf("\t%d: ", mitr->first);
        printThisSet(mitr->second);
        printf("\n");
    }
    fflush(stdout); 
}


void FairScheduler::printThisVector(vector<thrID> v) {
    vector<int>::iterator itr;
    printf("vector: <");
    for (itr = v.begin(); itr != v.end(); itr++) {
        printf("%d,", (*itr));
    }
    printf(">\n");
    fflush(stdout);

}
