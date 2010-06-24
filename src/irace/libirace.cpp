#include "libirace.h"

void IraceHandler::AfterInitialize() {
    global = new pthread_mutex_t;
    Originals::pthread_mutex_init(global, NULL);
    glock();
    main_thr = new pthread_t;
    (*main_thr) = Originals::pthread_self();
    tracker = new ThreadTracker();
    thrID myself = tracker->getNewTID(main_thr);
    ThreadInfo::set(new TLSIraceData(-1, myself));
    initIracer();
    gunlock();
}

void IraceHandler::ThreadStart(void * arg) {
    glock();
    thrID myself = getThrID();
    myrace->ThreadStart(myself);
    gunlock();
} 

void IraceHandler::ThreadFinish(void * status) {
    glock();
    thrID myself = getThrID();
    myrace->ThreadFinish(myself);
    gunlock();
} 

bool IraceHandler::BeforeCreate(pthread_t * param0, 
        const pthread_attr_t * param1,
        void *(* param2)(void *),
        void * param3,
        ThreadInfo * & param4) {
    glock();
    thrID child = tracker->getNewTID(param0);
    thrID myself = getThrID();
    param4 = new TLSIraceData(myself, child);
    myrace->BeforeCreate(myself, child);
    gunlock();
    return true;

}

void IraceHandler::AfterJoin(int ret_val, pthread_t thr, void** value) {
    glock();
    thrID myself = getThrID();
    thrID target = tracker->translateHandleToTID(&thr);
    myrace->AfterJoin(myself, target);
    gunlock();

}

void IraceHandler::AfterMutexLock(int ret_val, pthread_mutex_t * lock) {
    glock();
    thrID myself = getThrID();
    myrace->AfterMutexLock(myself, lock);
    gunlock();
}

bool IraceHandler::BeforeMutexUnlock(pthread_mutex_t * lock) {
    glock();
    thrID myself = getThrID();
    myrace->BeforeMutexUnlock(myself, lock);
    gunlock();
    return true;
}


bool IraceHandler::BeforeCondSignal(pthread_cond_t * cond) {
    glock();
    thrID myself = getThrID();
    myrace->BeforeCondSignal(myself, cond);
    gunlock();
    return true;
}

bool IraceHandler::BeforeCondBroadcast(pthread_cond_t * cond) {
    glock();
    thrID myself = getThrID();
    myrace->BeforeCondBroadcast(myself, cond);
    gunlock();
    return true;
}

void IraceHandler::AfterCondWait(int ret_val, pthread_cond_t * cond) {
    glock();
    thrID myself = getThrID();
    myrace->AfterCondWait(myself, cond);
    gunlock();
}

void IraceHandler::AfterCondTimedwait(int ret_val, pthread_cond_t * cond, 
        const struct timespec * time) {
    glock();
    thrID myself = getThrID();
    myrace->AfterCondTimedwait(myself, ret_val, cond);
    gunlock();

}

void IraceHandler::memoryWrite(int iid, void * addr) {
    glock();
    thrID myself = getThrID();
    myrace->memoryWrite(myself, iid, addr);
    gunlock();
}

void IraceHandler::memoryRead(int iid, void * addr) {
    glock();
    thrID myself = getThrID();
    myrace->memoryRead(myself, iid, addr);
    gunlock();
}

thrID IraceHandler::getThrID() {
    ThreadInfo * t = ThreadInfo::get();
    TLSIraceData * data = dynamic_cast<TLSIraceData *>(t);
    return data->me;
}

thrID IraceHandler::getThrParent(){
    ThreadInfo * t = ThreadInfo::get();
    TLSIraceData * data = dynamic_cast<TLSIraceData *>(t);
    return data->parent;
}

void IraceHandler::initIracer() {
    thrID myself = getThrID();
    myrace = new Iracer(myself);
}

void IraceHandler::glock() {
    Originals::pthread_mutex_lock(global);

}

void IraceHandler::gunlock() {
    Originals::pthread_mutex_unlock(global);
}

