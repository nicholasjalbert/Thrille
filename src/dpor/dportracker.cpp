#include "dportracker.h"

void DporTracker::incrementCurrentTransition() {
    currentTransition++;
}

int DporTracker::getCurrentTransition() {
    return currentTransition;
}


void DporTracker::dporThreadStart(thrID, void *) {

} 

void DporTracker::dporThreadFinish(thrID, void *) {

}

void DporTracker::dporBeforeCondTimedwait(thrID, 
                pthread_cond_t *, 
                pthread_mutex_t *,
                const struct timespec *) {

}

void DporTracker::dporAfterCondTimedwait(thrID myself,
                pthread_cond_t * cond, 
                pthread_mutex_t * lock,
                const struct timespec * time) {
    vc->afterWait(myself, cond);

}

void DporTracker::dporBeforeCreate(thrID myself,
                thrID child,
                pthread_t * handle, 
                const pthread_attr_t * attr,
                void *(*routine)(void *),
                void * arg,
                ThreadInfo * & tInfo) {
    vc->beforeCreate(myself, child);
}

void DporTracker::dporAfterCreate(thrID,
                int,
                pthread_t *,
                const pthread_attr_t *,
                void *(*)(void *),
                void * ) {

}

void DporTracker::dporAfterJoin(thrID myself,
        thrID target, pthread_t handle, void** arg) {
    vc->afterJoin(myself, target);
}

void DporTracker::dporBeforeMutexLock(thrID, pthread_mutex_t *) {

}

void DporTracker::dporSimulateMutexLock(thrID, pthread_mutex_t *) {

}

void DporTracker::dporAfterMutexUnlock(thrID, int, pthread_mutex_t *) {

}

void DporTracker::dporBeforeCondWait(thrID, 
                pthread_cond_t *, 
                pthread_mutex_t *) {

}

void DporTracker::dporAfterCondWait(thrID myself,
                pthread_cond_t * cond, 
                pthread_mutex_t * lock) {
    vc->afterWait(myself, cond);

}

void DporTracker::dporBeforeCondSignal(thrID myself,
        pthread_cond_t * cond) {
    vc->beforeSignal(myself, cond);
}

void DporTracker::dporSimulateCondSignal(thrID, pthread_cond_t *) {

}

void DporTracker::dporBeforeCondBroadcast(thrID myself,
        pthread_cond_t * cond) {
    vc->beforeSignal(myself, cond);
}

void DporTracker::dporSimulateCondBroadcast(thrID, pthread_cond_t *) {

}

void DporTracker::dpormyMemoryRead(thrID, int, void *) {

}

void DporTracker::dpormyMemoryWrite(thrID myself, int iid, void * addr) {
    vectorclock vector = vc->getVectorClock(myself);

}

void DporTracker::dporBeforeExit(thrID, void *) {

}

void DporTracker::dporSimulateExit(thrID, void *) {

}

void DporTracker::dpormyUsleep(thrID, useconds_t) {

}


