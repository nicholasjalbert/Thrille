#include "barriertracker.h"

BarrierTracker::BarrierTracker() {

}

BarrierTracker::~BarrierTracker() {

}

void BarrierTracker::initBarrier(pthread_barrier_t * bar, unsigned count) {
    vector<BarrierInfo>::iterator itr;
    for (itr = bar_info.begin(); itr != bar_info.end(); itr++) {
        BarrierInfo tmp = (*itr);
        if (tmp.barrier == bar) {
            itr->count = count;
            return;
        }
    }

    BarrierInfo tmp(bar, count);
    bar_info.push_back(tmp);
}

unsigned BarrierTracker::getBarrierCount(pthread_barrier_t * bar) {
    vector<BarrierInfo>::iterator itr;
    for (itr = bar_info.begin(); itr != bar_info.end(); itr++) {
        BarrierInfo tmp = (*itr);
        if (tmp.barrier == bar) {
            return tmp.count;
        }
    }
    printf("BarrierTracker: ERROR: could not find barrier count\n");
    _Exit(UNRECOVERABLE_ERROR);
    return -1;
}

void BarrierTracker::threadWaitingOnBarrier(thrID myself,
        pthread_barrier_t * bar) {
    thread_map[myself] = bar;
}

int BarrierTracker::getNumberThreadsWaitingOnBarrier(pthread_barrier_t * bar) {
    int number = 0;
    map<thrID, pthread_barrier_t *>::iterator itr;
    for (itr = thread_map.begin(); itr != thread_map.end(); itr++) {
        if (itr->second == bar) {
            number++;
        }
    }
    return number;
}

vector<thrID> BarrierTracker::getThreadsWaitingOnBarrier(
        pthread_barrier_t * bar) {
    vector<thrID> thr;
    map<thrID, pthread_barrier_t *>::iterator itr;
    for (itr = thread_map.begin(); itr != thread_map.end(); itr++) {
        if (itr->second == bar) {
            thr.push_back(itr->first);
        }
    }
    return thr;
}

void BarrierTracker::clearThreadsWaitingOnBarrier(pthread_barrier_t * bar) {
    map<thrID, pthread_barrier_t *>::iterator itr;
    for (itr = thread_map.begin(); itr != thread_map.end(); itr++) {
        if (itr->second == bar) {
            itr->second = NULL;
        }
    }
}



