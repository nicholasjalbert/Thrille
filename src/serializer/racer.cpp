#include "racer.h"

Racer::Racer(thrID main) {
    main_thr = main;
}

Racer::~Racer() {}

void Racer::ThreadStart(thrID) {

}

void Racer::ThreadFinish(thrID) {

}

void Racer::BeforeCreate(thrID, thrID) {

}

void Racer::AfterJoin(thrID, thrID) {

}

void Racer::AfterMutexLock(thrID, pthread_mutex_t *) {

}

void Racer::BeforeMutexUnlock(thrID, pthread_mutex_t *) {

}

void Racer::BeforeCondSignal(thrID, pthread_cond_t *) {

}

void Racer::BeforeCondBroadcast(thrID, pthread_cond_t *) {

}

void Racer::AfterCondWait(thrID, pthread_cond_t *) {

}

void Racer::AfterCondTimedwait(thrID, int, pthread_cond_t *) {

}

void Racer::memoryRead(thrID, void *, void *) {

}

void Racer::memoryWrite(thrID, void *, void *) {

}


