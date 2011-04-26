#include "threadtracker.h"

ThreadTracker::ThreadTracker() {
    curr_id = 0;
    Originals::pthread_mutex_init(&threadtracker_mutex, NULL);
}

ThreadTracker::~ThreadTracker() {}

/*
 * handle reuse is implemented by simply finding the 
 * highest TID that matches.  This is reasonable
 * because the if there is more than one match,
 * it means that another thread must have been 
 * spawned with the same handle.
 */
thrID ThreadTracker::translateHandleToTID(pthread_t id) {
    Originals::pthread_mutex_lock(&threadtracker_mutex);
    map<thrID, pthread_t>::iterator itr;
    thrID result = -1;
    //Originals::usleep(100);
    for (itr = pthreadHandleMap.begin(); 
            itr != pthreadHandleMap.end(); 
            itr++) {
        pthread_t tmp = itr->second;
        //printf("Threadtracker: tmp: %lu, id: %lu\n", tmp, id);
        if (Originals::pthread_equal(tmp, id)) {
            result = max(itr->first, result);
        }
    }
    Originals::pthread_mutex_unlock(&threadtracker_mutex);
    if (result == -1) {
        printf("ThreadTracker: ERROR: could not find TID -> handle map\n");
        _Exit(UNRECOVERABLE_ERROR);
    }
    return result;
}

thrID ThreadTracker::getNextTID() {
    Originals::pthread_mutex_lock(&threadtracker_mutex);
    curr_id++;
    Originals::pthread_mutex_unlock(&threadtracker_mutex);
    return curr_id;
}

void ThreadTracker::setTID(thrID id, pthread_t handle) {
    Originals::pthread_mutex_lock(&threadtracker_mutex);
    pthreadHandleMap[id] = handle; 
    Originals::pthread_mutex_unlock(&threadtracker_mutex);
}

