#include "libserializer.h"

void catch_sigseg(int sig) {
    SerializerHandler * myhandler = 
        dynamic_cast<SerializerHandler *>(pHandler);
    myhandler->thrilleSegfault();
}

SerializerHandler::SerializerHandler() : Handler() {
    struct sigaction segfault_catcher;
    memset(&segfault_catcher, 0, sizeof(segfault_catcher));
    segfault_catcher.sa_handler = catch_sigseg;
    sigaction(SIGSEGV, &segfault_catcher, NULL);
    printf("Thrille:Starting Serializer Thriller...\n");
    safe_mode = true;
}

SerializerHandler::~SerializerHandler() {
    delete controller;
    printf("Thrille:Ending Serializer Thriller...\n");
}

void SerializerHandler::AfterInitialize() {
    thrID myself = getThrID();
    race_detection_lock = new pthread_mutex_t;
    Originals::pthread_mutex_init(race_detection_lock, NULL);
    race = getNewRaceDetect(myself);
    controller = getNewExecutionTracker(myself);
}

void SerializerHandler::thrilleSegfault() {
    thrID myself = getThrID();
    controller->segfault(myself);
    _Exit(UNRECOVERABLE_ERROR);
}

bool SerializerHandler::BeforeGetschedparam(void * ret_addr, pthread_t arg0,
        int * arg1, struct sched_param  * arg2) {
    thrID myself = getThrID();
    return controller->BeforeGetschedparam(myself, ret_addr, arg0, arg1, arg2);
}


bool SerializerHandler::BeforeKeyCreate(void * ret_addr, pthread_key_t * key,
        void (*rt)(void*)) {
    thrID myself = getThrID();
    return controller->BeforeKeyCreate(myself, ret_addr, key, rt);
}

bool SerializerHandler::BeforeSetspecific(void * ret_addr, pthread_key_t arg0,
        const void * arg1) {
    thrID myself = getThrID();
    return controller->BeforeSetspecific(myself, ret_addr, arg0, arg1);
}

bool SerializerHandler::BeforeGetspecific(void * ret_addr,
        pthread_key_t arg0) {
    thrID myself = getThrID();
    return controller->BeforeGetspecific(myself, ret_addr, arg0);
}

bool SerializerHandler::BeforeAttrSetstacksize(void * ret_addr,
        pthread_attr_t * arg0, size_t arg1) {
    thrID myself = getThrID();
    return controller->BeforeAttrSetstacksize(myself, ret_addr, arg0, arg1);
}

bool SerializerHandler::BeforeAttrSetdetachstate(void * ret_addr,
        pthread_attr_t * arg0, int arg1) {
    thrID myself = getThrID();
    return controller->BeforeAttrSetdetachstate(myself, ret_addr, arg0, arg1);
}

bool SerializerHandler::BeforeAttrGetschedparam(void * ret_addr,
        const pthread_attr_t * arg0, 
        struct sched_param * arg1) {
    thrID myself = getThrID();
    return controller->BeforeAttrGetschedparam(myself, ret_addr, arg0, arg1);
}

bool SerializerHandler::BeforeAttrSetschedparam(void * ret_addr,
        pthread_attr_t * arg0,
        const struct sched_param * arg1) {
    thrID myself = getThrID();
    return controller->BeforeAttrSetschedparam(myself, ret_addr, arg0, arg1);
}


bool SerializerHandler::BeforeEqual(void * ret_addr,
        pthread_t arg0, pthread_t arg1) {
    thrID myself = getThrID();
    return controller->BeforeEqual(myself, ret_addr, arg0, arg1);
}

bool SerializerHandler::BeforeAttrDestroy(void * ret_addr,
        pthread_attr_t * arg0) {
    thrID myself = getThrID();
    return controller->BeforeAttrDestroy(myself, ret_addr, arg0);
}

bool SerializerHandler::BeforeMutexTrylock(void * ret_addr,
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    return controller->BeforeMutexTrylock(myself, ret_addr, lock);
}

int SerializerHandler::SimulateMutexTrylock(void * ret_addr,
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    return controller->SimulateMutexTrylock(myself, ret_addr, lock);
}

void SerializerHandler::AfterMutexTrylock(void * ret_addr, int ret,
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    controller->AfterMutexTrylock(myself, ret_addr, ret, lock);
}

bool SerializerHandler::BeforeMutexattrSettype(void * ret_addr,
                pthread_mutexattr_t * attr,
                int type) {
    thrID myself = getThrID();
    return controller->BeforeMutexattrSettype(myself, ret_addr, attr, type);
}


bool SerializerHandler::BeforeMutexattrInit(void * ret_addr,
        pthread_mutexattr_t * mattr) {
    thrID myself = getThrID();
    return controller->BeforeMutexattrInit(myself, ret_addr, mattr);
}

void SerializerHandler::thrilleAssert(void * ret_addr, bool cond) {
    thrID myself = getThrID();
    controller->thrilleAssert(myself, ret_addr, cond);
}

void SerializerHandler::thrilleSchedPoint(void * ret_addr) {
    thrID myself = getThrID();
    controller->thrilleSchedPoint(myself, ret_addr);
}

void SerializerHandler::thrilleCheckpoint(void * ret_addr) {
    thrID myself = getThrID();
    controller->thrilleCheckpoint(myself, ret_addr);
}


bool SerializerHandler::BeforeSetcanceltype(void * ret_addr,
        int param0, int * param1) {
    thrID myself = getThrID();
    return controller->BeforeSetcanceltype(myself, ret_addr, param0, param1);
}

void SerializerHandler::ThreadStart(void * (*start_routine) (void *), 
        void * arg) {
    thrID myself = getThrID();
    controller->ThreadStart(myself, start_routine, arg);

    Originals::pthread_mutex_lock(race_detection_lock);
    race->ThreadStart(myself);
    Originals::pthread_mutex_unlock(race_detection_lock);
} 

void SerializerHandler::ThreadFinish(void * (*start_routine) (void *),
        void * status) {
    thrID myself = getThrID();
    controller->ThreadFinish(myself, start_routine, status);
    
    Originals::pthread_mutex_lock(race_detection_lock);
    race->ThreadFinish(myself);
    Originals::pthread_mutex_unlock(race_detection_lock);
}


bool SerializerHandler::BeforeBarrierDestroy(void * ret_addr,
        pthread_barrier_t * barrier) {
    thrID myself = getThrID();
    return controller->BeforeBarrierDestroy(myself, ret_addr, barrier);
}

int SerializerHandler::SimulateBarrierDestroy(void * ret_addr,
        pthread_barrier_t * barrier) {
    thrID myself = getThrID();
    return controller->SimulateBarrierDestroy(myself, ret_addr, barrier);
}


bool SerializerHandler::BeforeOnce(void * ret_addr, 
        pthread_once_t * once, void (* param1)(void)) {
    thrID myself = getThrID();
    return controller->BeforeOnce(myself, ret_addr, once, param1);
}

bool SerializerHandler::BeforeMutexInit(void * ret_addr,
        pthread_mutex_t * lock, const pthread_mutexattr_t * attr) {
    thrID myself = getThrID();
    return controller->BeforeMutexInit(myself, ret_addr, lock, attr);
}

bool SerializerHandler::BeforeCondInit(void * ret_addr, pthread_cond_t * cond,
        const pthread_condattr_t * attr) {
    thrID myself = getThrID();
    return controller->BeforeCondInit(myself, ret_addr, cond, attr);
}


bool SerializerHandler::BeforeSelf(void * ret_addr) {
    thrID myself = getThrID();
    return controller->BeforeSelf(myself, ret_addr);
}

bool SerializerHandler::BeforeAttrInit(void * ret_addr, pthread_attr_t * at) {
    thrID myself = getThrID();
    return controller->BeforeAttrInit(myself, ret_addr, at);
}


bool SerializerHandler::BeforeAttrSetscope(void * ret_addr,
        pthread_attr_t * attr,
        int scope) {
    thrID myself = getThrID();
    return controller->BeforeAttrSetscope(myself, ret_addr, attr, scope);
}

bool SerializerHandler::BeforeSemDestroy(void * ret_addr, sem_t * sem) {
    thrID myself = getThrID();
    return controller->BeforeSemDestroy(myself, ret_addr, sem);
}


int SerializerHandler::SimulateSemDestroy(void * ret_addr, sem_t * sem) {
    thrID myself = getThrID();
    return controller->SimulateSemDestroy(myself, ret_addr, sem);
}

bool SerializerHandler::BeforeMutexDestroy(void * ret_addr, 
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    return controller->BeforeMutexDestroy(myself, ret_addr, lock);

}

int SerializerHandler::SimulateMutexDestroy(void * ret_addr,
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    return controller->SimulateMutexDestroy(myself, ret_addr, lock);
}

bool SerializerHandler::BeforeCondDestroy(void * ret_addr,
        pthread_cond_t * cond) {
    thrID myself = getThrID();
    return controller->BeforeCondDestroy(myself, ret_addr, cond);
}

int SerializerHandler::SimulateCondDestroy(void * ret_addr,
        pthread_cond_t * cond) {
    thrID myself = getThrID();
    return controller->SimulateCondDestroy(myself, ret_addr, cond);
}

bool SerializerHandler::BeforeCreate(void * ret_addr, pthread_t * param0, 
        const pthread_attr_t * param1,
        void *(* param2)(void *),
        void * param3,
        thrID & child) {
    thrID myself = getThrID(); 
    bool result = controller->BeforeCreate(myself,
            child,
            ret_addr,
            param0,
            param1,
            param2,
            param3);

    Originals::pthread_mutex_lock(race_detection_lock);
    race->BeforeCreate(myself, child);
    Originals::pthread_mutex_unlock(race_detection_lock);
    return result;
}

void SerializerHandler::AfterCreate(void * ret_addr, int ret_val,
        pthread_t * param0,
        const pthread_attr_t * param1,
        void *(* param2)(void *),
        void * param3,
        thrID & child) { 
    thrID myself = getThrID();
    return controller->AfterCreate(myself, 
            child,
            ret_addr,
            ret_val, 
            param0,
            param1,
            param2,
            param3);
}

bool SerializerHandler::BeforeJoin(void * ret_addr,
        pthread_t thread, void** val) {
    thrID myself = getThrID();
    thrID target = translateHandleToTID(thread);
    return controller->BeforeJoin(myself, target, ret_addr, thread, val);
}

void SerializerHandler::AfterJoin(void * ret_addr, int ret,
        pthread_t thread, void ** val) {
    /* needed for race detection but can result in segfault
     * if a thread is removed from the tracker before this is
     * called.
     *
     * thrID myself = getThrID();
     * thrID target = translateHandleToTID(thread);
     * Originals::pthread_mutex_lock(race_detection_lock);
     * race->AfterJoin(myself, target);
     * Originals::pthread_mutex_unlock(race_detection_lock);
     */
}

bool SerializerHandler::BeforeMutexLock(void * ret_addr,
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    return controller->BeforeMutexLock(myself, ret_addr, 
            lock, false);
}

int SerializerHandler::SimulateMutexLock(void * ret_addr,
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    return controller->SimulateMutexLock(myself, ret_addr, lock, false);
}

void SerializerHandler::AfterMutexLock(void * ret_addr, int ret,
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    
    Originals::pthread_mutex_lock(race_detection_lock);
    race->AfterMutexLock(myself, lock);
    Originals::pthread_mutex_unlock(race_detection_lock);
}


bool SerializerHandler::BeforeMutexUnlock(void * ret_addr,
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    
    Originals::pthread_mutex_lock(race_detection_lock);
    race->BeforeMutexUnlock(myself, lock);
    Originals::pthread_mutex_unlock(race_detection_lock);
    return controller->BeforeMutexUnlock(myself, ret_addr, lock, false);
}

void SerializerHandler::AfterMutexUnlock(void * ret_addr, int ret_val, 
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    return controller->AfterMutexUnlock(myself, ret_addr, ret_val,
            lock, false);
}

bool SerializerHandler::BeforeCondWait(void * ret_addr, pthread_cond_t * cond, 
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    return controller->BeforeCondWait(myself, ret_addr, cond,
            lock);
}

int SerializerHandler::SimulateCondWait(void * ret_addr,
        pthread_cond_t * cond, 
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    return controller->SimulateCondWait(myself, ret_addr, cond,
            lock);
}

void SerializerHandler::AfterCondWait(void * ret_addr, int ret,
        pthread_cond_t * cond, 
        pthread_mutex_t * lock) {
    thrID myself = getThrID();
    Originals::pthread_mutex_lock(race_detection_lock);
    race->AfterCondWait(myself, cond);
    Originals::pthread_mutex_unlock(race_detection_lock);
}


bool SerializerHandler::BeforeCondTimedwait(void * ret_addr,
        pthread_cond_t * cond, 
        pthread_mutex_t * lock, 
        const struct timespec * time) {
    thrID myself = getThrID();
    return controller->BeforeCondTimedwait(myself, ret_addr, cond, lock,
            time);
}

int SerializerHandler::SimulateCondTimedwait(void * ret_addr,
        pthread_cond_t * cond, 
        pthread_mutex_t * lock,
        const struct timespec * time) {
    thrID myself = getThrID();
    return controller->SimulateCondTimedwait(myself, ret_addr, cond, lock,
            time);
}

void SerializerHandler::AfterCondTimedwait(void * ret_addr, int ret,
        pthread_cond_t * cond, 
        pthread_mutex_t * lock, 
        const struct timespec * time) {
    thrID myself = getThrID();
    Originals::pthread_mutex_lock(race_detection_lock);
    race->AfterCondTimedwait(myself, ret, cond); 
    Originals::pthread_mutex_unlock(race_detection_lock);
}

bool SerializerHandler::BeforeCondSignal(void * ret_addr,
        pthread_cond_t * cond) {
    thrID myself = getThrID();
    bool ret = controller->BeforeCondSignal(myself, ret_addr, cond);
    Originals::pthread_mutex_lock(race_detection_lock);
    race->BeforeCondSignal(myself, cond);
    Originals::pthread_mutex_unlock(race_detection_lock);
    return ret;
}

int SerializerHandler::SimulateCondSignal(void * ret_addr,
        pthread_cond_t * cond) {
    thrID myself = getThrID();
    return controller->SimulateCondSignal(myself, ret_addr, cond);
}

bool SerializerHandler::BeforeCondBroadcast(void * ret_addr,
        pthread_cond_t * cond) {
    thrID myself = getThrID();
    bool ret = controller->BeforeCondBroadcast(myself, ret_addr, cond);
    Originals::pthread_mutex_lock(race_detection_lock);
    race->BeforeCondBroadcast(myself, cond);
    Originals::pthread_mutex_unlock(race_detection_lock);
    return ret;
}

int SerializerHandler::SimulateCondBroadcast(void * ret_addr,
        pthread_cond_t * cond) {
    thrID myself = getThrID();
    return controller->SimulateCondBroadcast(myself, ret_addr, cond);
}

bool SerializerHandler::BeforeBarrierInit(void * ret_addr,
        pthread_barrier_t * barrier,
        const pthread_barrierattr_t * battr, unsigned count) {
    thrID myself = getThrID();
    return controller->BeforeBarrierInit(myself, ret_addr, barrier, 
            battr, count);
}

bool SerializerHandler::BeforeBarrierWait(void * ret_addr,
        pthread_barrier_t * barrier) {
    thrID myself = getThrID();
    return controller->BeforeBarrierWait(myself, ret_addr, barrier);
}


void SerializerHandler::myMemoryRead(void * ret_addr, void * addr) {
    thrID myself = getThrID();
    controller->myMemoryRead(myself, ret_addr, addr);
    Originals::pthread_mutex_lock(race_detection_lock);
    race->memoryRead(myself, ret_addr, addr);
    Originals::pthread_mutex_unlock(race_detection_lock);
}

void SerializerHandler::myMemoryWrite(void * ret_addr, void * addr) {
    thrID myself = getThrID();
    controller->myMemoryWrite(myself, ret_addr, addr);
    Originals::pthread_mutex_lock(race_detection_lock);
    race->memoryWrite(myself, ret_addr, addr);
    Originals::pthread_mutex_unlock(race_detection_lock);
}

bool SerializerHandler::BeforeSleep(void * ret_addr, unsigned int time) {
    thrID myself = getThrID();
    return controller->BeforeSleep(myself, ret_addr, time);
}

unsigned int SerializerHandler::SimulateSleep(void * ret_addr,
        unsigned int time) {
    thrID myself = getThrID();
    return controller->SimulateSleep(myself, ret_addr, time);
}

bool SerializerHandler::BeforeSigwait(void * ret_addr, const sigset_t * set,
        int * sig){
    thrID myself = getThrID();
    return controller->BeforeSigwait(myself, ret_addr, set, sig);
}

bool SerializerHandler::BeforeUsleep(void * ret_addr, useconds_t time) {
    thrID myself = getThrID();
    return controller->BeforeUsleep(myself, ret_addr, time);
}

int SerializerHandler::SimulateUsleep(void * ret_addr, useconds_t time) {
    thrID myself = getThrID();
    return controller->SimulateUsleep(myself, ret_addr, time);
}


bool SerializerHandler::BeforeSemWait(void * ret_addr, sem_t * sem) {
    thrID myself = getThrID();
    return controller->BeforeSemWait(myself, ret_addr, sem);
}

int SerializerHandler::SimulateSemWait(void * ret_addr, sem_t * sem) {
    thrID myself = getThrID();
    return controller->SimulateSemWait(myself, ret_addr, sem);
}

bool SerializerHandler::BeforeSemPost(void * ret_addr, sem_t * sem) {
    thrID myself = getThrID();
    return controller->BeforeSemPost(myself, ret_addr, sem);
}

int SerializerHandler::SimulateSemPost(void * ret_addr, sem_t * sem) {
    thrID myself = getThrID();
    return controller->SimulateSemPost(myself, ret_addr, sem);
}

bool SerializerHandler::BeforeSemInit(void * ret_addr, sem_t * sem,
        int pshared, unsigned int value) {
    thrID myself = getThrID();
    return controller->BeforeSemInit(myself, ret_addr, sem, pshared, value);
}


ExecutionTracker * SerializerHandler::getNewExecutionTracker(thrID myself) {
    return new ExecutionTracker(myself);
}

bool SerializerHandler::BeforeExit(void * ret_addr, void * arg) {
    thrID myself = getThrID();
    return controller->BeforeExit(myself, ret_addr, arg);
}

void SerializerHandler::SimulateExit(void * ret_addr, void * arg) {
    thrID myself = getThrID();
    controller->SimulateExit(myself, ret_addr, arg);
}

Racer * SerializerHandler::getNewRaceDetect(thrID myself) {
    return new Racer(myself);
}

bool SerializerHandler::isSchedulingPoint(void * ret_addr) {
    return controller->isSchedulingPoint(ret_addr);
}

bool SerializerHandler::tryToSchedule() {
    return controller->tryToSchedule();
}

void myMemoryRead(int iid, void * addr) {
    SerializerHandler * myhandler = 
        dynamic_cast<SerializerHandler *>(pHandler);
    if (myhandler->controller->log->tryToSchedule()) {
        void * my_id = __builtin_return_address(0);
        //Does this need a lock?
        if (myhandler->isSchedulingPoint(my_id)) {
            myhandler->myMemoryRead(my_id, addr);
        }
    }
}

void myMemoryWrite(int iid, void * addr) {
    SerializerHandler * myhandler = 
        dynamic_cast<SerializerHandler *>(pHandler);
    if (myhandler->controller->log->tryToSchedule()) {
        void * my_id = __builtin_return_address(0);
        //Does this need a lock?
        if (myhandler->isSchedulingPoint(my_id)) {
            myhandler->myMemoryWrite(my_id, addr);
        }
    }
}

void thrilleSchedPointC() {
    SerializerHandler * myhandler = 
        dynamic_cast<SerializerHandler *>(pHandler);
    void * my_id = __builtin_return_address(0);
    myhandler->thrilleSchedPoint(my_id);
}

void thrilleSchedPointCPP() {
    SerializerHandler * myhandler = 
        dynamic_cast<SerializerHandler *>(pHandler);
    void * my_id = __builtin_return_address(0);
    myhandler->thrilleSchedPoint(my_id);
}

void thrilleAssertC(int cond) {
    SerializerHandler * myhandler = 
        dynamic_cast<SerializerHandler *>(pHandler);
    void * my_id = __builtin_return_address(0);
    if (cond == 0) {
        myhandler->thrilleAssert(my_id, false);
    } else {
        myhandler->thrilleAssert(my_id, true);
    }
}

void thrilleAssertCPP(bool cond) {
    SerializerHandler * myhandler = 
        dynamic_cast<SerializerHandler *>(pHandler);
    void * my_id = __builtin_return_address(0);
    myhandler->thrilleAssert(my_id, cond);
}

void thrilleCheckpointC() {
    SerializerHandler * myhandler = 
        dynamic_cast<SerializerHandler *>(pHandler);
    void * my_id = __builtin_return_address(0);
    myhandler->thrilleCheckpoint(my_id);
}

void thrilleCheckpointCPP() {
    SerializerHandler * myhandler = 
        dynamic_cast<SerializerHandler *>(pHandler);
    void * my_id = __builtin_return_address(0);
    myhandler->thrilleCheckpoint(my_id);
}


void * thrilleMallocC(size_t size) {
    void * the_ret = malloc(size);
    memset(the_ret, UCHAR_MAX, size); 
    return the_ret;
}

void * thrilleMallocCPP(size_t size) {
    void * the_ret = malloc(size);
    memset(the_ret, UCHAR_MAX, size); 
    return the_ret;
}
