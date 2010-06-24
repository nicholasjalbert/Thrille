#include "libracer.h"

RacerHandler::RacerHandler() : Handler() {
    printf("Starting Race detection...\n");

    thrID myself = getThrID();

    finish_map[myself] = false;
    
    unsigned int wr_profile_size = 5;
    unsigned int rd_profile_size = 5;
    eb = new HybridRaceTracker(wr_profile_size, rd_profile_size);
    printf("Write profiling size is %d\n", wr_profile_size);
    printf("Read profiling size is %d\n", rd_profile_size);
}

RacerHandler::~RacerHandler() {
    eb->outputRaces();
    delete eb;
    printf("Ending Race detection...\n");
}


void RacerHandler::initializeThreadData(thrID thread) {
    finish_map[thread] = false;
}

bool RacerHandler::BeforeCreate(void * ret_addr,
        pthread_t* newthread, 
        const pthread_attr_t *attr,
        void *(*start_routine) (void *), 
        void *arg, 
        thrID & child) {
    lock();

    initializeThreadData(child);
    eb->beforeCreate(getThrID(), child);
    
    unlock();
    return true;
} 


void RacerHandler::AfterJoin(void * ret_addr,
        int ret_val,
        pthread_t th,
        void ** thread_return) {
    lock();
    
    thrID join_target = translateHandleToTID(th);
    eb->afterJoin(getThrID(), join_target);

    unlock();
}

bool RacerHandler::BeforeMutexLock(void * ret_addr, pthread_mutex_t * mutex) {
    lock();
    
    lockRaceEvent tmp (ret_addr, getThrID(), mutex, false, 
            eb->getVectorClock(getThrID()), eb->getLockSet(getThrID()));
    eb->addLockEvent(tmp);
    eb->lockBefore(getThrID(), mutex);

    unlock();
    return true;

}

void RacerHandler::AfterMutexUnlock(void * ret_addr, int ret_val,
        pthread_mutex_t * mutex) {
    lock();

    eb->unlockAfter(getThrID(), mutex);
    
    unlock();
}
	
void RacerHandler::AfterCondWait(void * ret_addr, int ret_val, 
                                    pthread_cond_t * cond,
                                    pthread_mutex_t * mutex) {
    lock();

    eb->afterWait(getThrID(), cond);
    
    unlock();
}

void RacerHandler::AfterCondTimedwait(void * ret_addr, int ret_val,
        pthread_cond_t * cond,
        pthread_mutex_t * mutex,
        const struct timespec * time) {
    lock();
    
    /* ensure it wasn't timeout */
    if (ret_val == 0) {
        eb->afterWait(getThrID(), cond);
    } 

    unlock();
}

bool RacerHandler::BeforeCondSignal(void * ret_addr, pthread_cond_t * cond) {
    lock();
    
    eb->beforeSignal(getThrID(), cond);
    
    unlock();
    return true;
}

bool RacerHandler::BeforeCondBroadcast(void * ret_addr, pthread_cond_t * cond){
    lock();
    
    eb->beforeSignal(getThrID(), cond);
    
    unlock();
    return true;
}

void RacerHandler::ThreadFinish(void * (*start) (void *), void * status) {
    if (finish_map[getThrID()]) {
        safe_assert(false);
    }
    finish_map[getThrID()] = true;
}

void RacerHandler::myMemoryRead(void * iid, void * addr) {
    lock();
    
    thrID myself = getThrID();
    dataRaceEvent r(iid, 
            myself,
            addr, 
            true, 
            eb->getVectorClock(myself), 
            eb->getLockSet(myself));
    eb->addEvent(r);

    unlock();
}

void RacerHandler::myMemoryWrite(void * iid, void * addr) {
    lock();

    thrID myself = getThrID();
    dataRaceEvent r(iid, 
            myself,
            addr, 
            false, 
            eb->getVectorClock(myself), 
            eb->getLockSet(myself));
    eb->addEvent(r);

    unlock();
}

bool RacerHandler::BeforeExit(void * ret_addr, void * value) {
    lock();
    ThreadFinish(NULL, 0);  
    unlock();
    return true;
}


