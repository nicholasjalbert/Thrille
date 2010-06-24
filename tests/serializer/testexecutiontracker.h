// Tests src/serializer/executiontracker
// using cxxtest

#include "../cxxtest/cxxtest/TestSuite.h"
#include "../../src/serializer/executiontracker.h"
#include "../../src/serializer/threadtracker.h"

void * threadSimulateCondWait(void * arg);
void * threadWakeThreadWaitingOnCond(void * arg);
void * threadSignalThreadReadyA(void * arg); 
void * threadSignalThreadReadyB(void * arg);

struct structSimulateCondWait {
    ExecutionTracker * my_t;
    pthread_mutex_t * my_mutex;
    pthread_cond_t * my_cond;
    thrID myself;

    structSimulateCondWait(ExecutionTracker * p1, 
            pthread_mutex_t * p2, 
            pthread_cond_t * p3,
            thrID p4) {
        my_t = p1;
        my_mutex = p2;
        my_cond = p3;
        myself = p4; 
    }
};


class ExecutionTrackerTestSuite : public CxxTest::TestSuite {

    public:
        ExecutionTracker * t;
        ThreadTracker * namer;
        pthread_t * thread;
        pthread_t * thread2;
        pthread_mutex_t * mutex;
        pthread_mutex_t * mutex2;
        pthread_cond_t * cond;
        pthread_cond_t * cond2;
        bool flag;

        void setUp() {
            flag = true;
            namer = new ThreadTracker();
            t = new ExecutionTracker();
            thread = new pthread_t;
            thread2 = new pthread_t;
            mutex = new pthread_mutex_t;
            mutex2 = new pthread_mutex_t;
            cond = new pthread_cond_t;
            cond2 = new pthread_cond_t;
            Originals::pthread_mutex_init(mutex, NULL);
            Originals::pthread_mutex_init(mutex2, NULL);
            Originals::pthread_cond_init(cond, NULL);
            Originals::pthread_cond_init(cond2, NULL);
        }

        void tearDown() {
            Originals::pthread_cond_destroy(cond2);
            Originals::pthread_cond_destroy(cond);
            Originals::pthread_mutex_destroy(mutex2);
            Originals::pthread_mutex_destroy(mutex);
            delete cond2;
            delete cond;
            delete mutex2;
            delete mutex;
            delete thread2;
            delete thread;
            delete t;
            delete namer;
            flag = true;
        }
        
        int getSemVal(sem_t * thesem) {
            int * semval;
            int val = -100;
            semval = &val;
            sem_getvalue(thesem, semval);
            return (*semval);
        }


        void testSchedule(void) {
            TS_ASSERT_THROWS_ANYTHING(t->schedule());
            
            thrID disabled_target = namer->getNewTID(thread);
            t->initializeThreadData(disabled_target);
            t->disableThread(disabled_target);
            TS_ASSERT_THROWS_ANYTHING(t->schedule());
            
            thrID enabled_target = namer->getNewTID(thread);
            t->initializeThreadData(enabled_target);
            t->enableThread(enabled_target);
            thrID result;
            TS_ASSERT_THROWS_NOTHING(result = t->schedule());
            TS_ASSERT(result == enabled_target);
        }

        void testEnableAndDisable(void) {
            thrID target =  namer->getNewTID(thread);
            t->enableThread(target);
            TS_ASSERT(t->isEnabled(target));
            t->disableThread(target);
            TS_ASSERT(t->isDisabled(target));
        }
        
        void testThreadStart(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            TS_ASSERT(getSemVal(t->sem_map[target]) == 0);

            t->wakeThread(target);
            TS_ASSERT(getSemVal(t->sem_map[target])== 1);

            t->ThreadStart(target, NULL);
            TS_ASSERT(getSemVal(t->sem_map[target]) == 0);
        }

        void testInitialization(void) {
            TS_ASSERT(Originals::pthread_mutex_lock(t->global_lock) == 0);
            TS_ASSERT(Originals::pthread_mutex_unlock(t->global_lock) == 0);
            TS_ASSERT(t->log != NULL);
        }

        void testInitializeThreadData(void) {
            thrID testbed = namer->getNewTID(thread);
            t->initializeThreadData(testbed); 
            TS_ASSERT(t->sem_map[testbed] != NULL);
            TS_ASSERT(sem_post(t->sem_map[testbed]) == 0);
            TS_ASSERT(sem_wait(t->sem_map[testbed]) == 0);

            TS_ASSERT(t->wait_sem_map[testbed] != NULL);
            TS_ASSERT(sem_post(t->wait_sem_map[testbed]) == 0);
            TS_ASSERT(sem_wait(t->wait_sem_map[testbed]) == 0);

            TS_ASSERT(t->isEnabled(testbed));
            TS_ASSERT(!t->finish_map[testbed]);
            TS_ASSERT(!t->exit_map[testbed]);
            TS_ASSERT(t->join_map[testbed] == -1);
        }

        void testPauseAndWake(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            TS_ASSERT(getSemVal(t->sem_map[target]) == 0);

            TS_ASSERT_THROWS_NOTHING(t->wakeThread(target));
            TS_ASSERT(getSemVal(t->sem_map[target]) == 1);
            TS_ASSERT_THROWS_NOTHING(t->pauseThread(target));
            TS_ASSERT(getSemVal(t->sem_map[target]) == 0);

            TS_ASSERT_THROWS_NOTHING(t->wakeThread(target));
            TS_ASSERT(getSemVal(t->sem_map[target]) == 1);
            TS_ASSERT_THROWS_NOTHING(t->wakeThread(target));
            TS_ASSERT(getSemVal(t->sem_map[target]) == 2);
            TS_ASSERT_THROWS_NOTHING(t->pauseThread(target));
            TS_ASSERT(getSemVal(t->sem_map[target]) == 1);
            TS_ASSERT_THROWS_NOTHING(t->pauseThread(target));
            TS_ASSERT(getSemVal(t->sem_map[target]) == 0);
        }

        void testGlobalLock(void) {
            TS_ASSERT_THROWS_NOTHING(t->glock());
        }

        void testGlobalUnlock(void) {
            TS_ASSERT_THROWS_NOTHING(t->glock());
            TS_ASSERT_THROWS_NOTHING(t->gunlock());
            TS_ASSERT_THROWS_ANYTHING(t->gunlock());
        }

        void testFinalizeThread(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);

            TS_ASSERT_THROWS_ANYTHING(t->finalizeThread(target));
            TS_ASSERT(t->isDisabled(target));
            TS_ASSERT(t->finish_map[target]);
        }

        void testThreadFinish(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            TS_ASSERT_THROWS_ANYTHING(t->ThreadFinish(target, NULL));
            TS_ASSERT(t->isDisabled(target));
            TS_ASSERT(t->finish_map[target]);
        }

        void testBeforeCreate(void) {
            thrID parent = namer->getNewTID(thread);
            thrID child = namer->getNewTID(thread2);
            ThreadInfo * param4 = new TLSSerializerData(child);
            t->BeforeCreate(parent, child, NULL, NULL, NULL, NULL, param4);
            TS_ASSERT(t->sem_map[child] != NULL);
            TS_ASSERT(sem_post(t->sem_map[child]) == 0);
            TS_ASSERT(sem_wait(t->sem_map[child]) == 0);

            TS_ASSERT(t->wait_sem_map[child] != NULL);
            TS_ASSERT(sem_post(t->wait_sem_map[child]) == 0);
            TS_ASSERT(sem_wait(t->wait_sem_map[child]) == 0);

            TS_ASSERT(t->isEnabled(child));
            TS_ASSERT(!t->finish_map[child]);
            TS_ASSERT(!t->exit_map[child]);
            TS_ASSERT(t->join_map[child] == -1);
        }

        void testAfterCreate(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            int ret_val = 0;
            TS_ASSERT(getSemVal(t->sem_map[target]) == 0);
            TS_ASSERT_THROWS_NOTHING(t->AfterCreate(target, 
                        ret_val, NULL, NULL, NULL, NULL));

            TS_ASSERT(getSemVal(t->sem_map[target]) == 0);
        }

        void testBeforeJoinA(void) {
            thrID parent = namer->getNewTID(thread);
            thrID child = namer->getNewTID(thread2);
            t->initializeThreadData(parent);
            t->initializeThreadData(child);
            TS_ASSERT(t->isEnabled(parent));
            TS_ASSERT(t->isEnabled(child));
            t->wakeThread(parent);
            TS_ASSERT(t->join_map[parent] == -1);
            t->BeforeJoin(parent, child, (*thread2), NULL);
            TS_ASSERT(t->join_map[child] == parent);
            TS_ASSERT(t->isDisabled(parent));
            t->finalizeThread(child);
            TS_ASSERT(t->isEnabled(parent));
        }
        
        void testBeforeJoinB(void) {
            thrID parent = namer->getNewTID(thread);
            thrID child = namer->getNewTID(thread2);
            t->initializeThreadData(parent);
            t->initializeThreadData(child);
            TS_ASSERT(t->isEnabled(parent));
            TS_ASSERT(t->isEnabled(child));
            t->wakeThread(parent);
            t->finalizeThread(child);
            TS_ASSERT(t->join_map[parent] == -1);
            t->BeforeJoin(parent, child, (*thread2), NULL);
            TS_ASSERT(t->join_map[child] == -1);
            TS_ASSERT(t->isEnabled(parent));
        }

        void testRegisterThreadJoinA(void) {
            TS_ASSERT_THROWS_ANYTHING(t->registerThreadJoin(-1, -1));
        }
        
        void testRegisterThreadJoinB(void) {
            thrID parent = namer->getNewTID(thread);
            thrID child = namer->getNewTID(thread2);
            t->initializeThreadData(parent);
            t->initializeThreadData(child);
            TS_ASSERT(t->join_map[parent] == -1);
            t->registerThreadJoin(child, parent);
            TS_ASSERT(t->join_map[child] == parent);
        }

        void testThreadIsFinished(void) {
            thrID parent = namer->getNewTID(thread);
            thrID child = namer->getNewTID(thread2);
            t->initializeThreadData(parent);
            t->initializeThreadData(child);
            TS_ASSERT(t->threadIsFinished(parent) == false);
            t->finalizeThread(parent);
            TS_ASSERT(t->threadIsFinished(parent));
        }

        void testEnableJoiningThread(void) {
            thrID parent = namer->getNewTID(thread);
            thrID child = namer->getNewTID(thread2);
            t->initializeThreadData(parent);
            t->initializeThreadData(child);
            t->disableThread(parent);
            t->disableThread(child);
            t->enableJoiningThread(child);
            TS_ASSERT(t->isDisabled(parent));
            TS_ASSERT(t->isDisabled(child));
            t->registerThreadJoin(child, parent);
            t->enableJoiningThread(child);
            TS_ASSERT(t->isEnabled(parent));
        }

        void testAddThreadWaitingOnLock(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            TS_ASSERT(t->lock_wait_map[target] == NULL);
            t->addThreadWaitingOnLock(target, mutex);
            TS_ASSERT(t->lock_wait_map[target] == mutex);
        }

        void testAddThreadReadyOnLock(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            TS_ASSERT(t->lock_ready_map[target] == NULL);
            t->addThreadReadyOnLock(target, mutex);
            TS_ASSERT(t->lock_ready_map[target] == mutex);
        }

        void testThreadHasAcquiredLock(void) {
            pthread_t * thread3 = new pthread_t;
            thrID target = namer->getNewTID(thread);
            thrID target2 = namer->getNewTID(thread2);
            thrID target3 = namer->getNewTID(thread3);
            t->initializeThreadData(target);
            t->initializeThreadData(target2);
            t->initializeThreadData(target3);
            t->addThreadReadyOnLock(target, mutex);
            t->addThreadReadyOnLock(target2, mutex);
            t->addThreadReadyOnLock(target3, mutex2);
            TS_ASSERT(t->isEnabled(target3));
            t->threadHasAcquiredLock(target, mutex);
            TS_ASSERT(t->lock_ready_map[target] == NULL);
            TS_ASSERT(t->lock_ready_map[target2] == NULL);
            TS_ASSERT(t->lock_ready_map[target3] == mutex2);
            TS_ASSERT(t->isEnabled(target));
            TS_ASSERT(t->isDisabled(target2));
            TS_ASSERT(t->isEnabled(target3));
            TS_ASSERT(t->lock_wait_map[target] == NULL);
            TS_ASSERT(t->lock_wait_map[target2] == mutex);
            TS_ASSERT(t->lock_wait_map[target3] == NULL);
            delete thread3;
        }

        void testThreadHasReleasedLock(void) {
            pthread_t * thread3 = new pthread_t;
            thrID target = namer->getNewTID(thread);
            thrID target2 = namer->getNewTID(thread2);
            thrID target3 = namer->getNewTID(thread3);
            t->initializeThreadData(target);
            t->initializeThreadData(target2);
            t->initializeThreadData(target3);
            t->addThreadWaitingOnLock(target2, mutex);
            t->addThreadWaitingOnLock(target3, mutex2);
            t->disableThread(target2); 
            t->disableThread(target3);
            t->threadHasReleasedLock(target, mutex);
            TS_ASSERT(t->isEnabled(target2));
            TS_ASSERT(t->lock_ready_map[target2] == mutex);
            TS_ASSERT(t->lock_wait_map[target2] == NULL);
            TS_ASSERT(t->isDisabled(target3));
            TS_ASSERT(t->lock_ready_map[target3] == NULL);
            TS_ASSERT(t->lock_wait_map[target3] == mutex2);
        }

        void testBeforeMutexLockA(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            t->wakeThread(target);
            t->BeforeMutexLock(target, mutex, (void *) 0x64);
            TS_ASSERT(Originals::pthread_mutex_trylock(mutex) == 0);
            Originals::pthread_mutex_unlock(mutex);
            TS_ASSERT(t->isEnabled(target));
            TS_ASSERT(t->lock_ready_map[target] == mutex);
        }
        
        void testBeforeMutexLockB(void) {
            thrID target = namer->getNewTID(thread);
            thrID target2 = namer->getNewTID(thread2);
            t->initializeThreadData(target);
            t->initializeThreadData(target2);
            t->wakeThread(target);
            Originals::pthread_mutex_lock(mutex);
            t->BeforeMutexLock(target, mutex, (void *) 0x64);
            TS_ASSERT(t->isDisabled(target));
            TS_ASSERT(t->lock_wait_map[target] == mutex);
            Originals::pthread_mutex_unlock(mutex);
        }

        void testBeforeMutexLockC(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            t->wakeThread(target);
            TS_ASSERT_THROWS_ANYTHING(t->BeforeMutexLock(target, 
                        NULL, (void *) 0x64));
        }

        void testSimulateMutexLock(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            TS_ASSERT(t->SimulateMutexLock(target, mutex) == 0);
            TS_ASSERT(Originals::pthread_mutex_trylock(mutex) == EBUSY);
        }

        void testAfterMutexUnlock(void) {
            thrID target = namer->getNewTID(thread);
            thrID target2 = namer->getNewTID(thread2);
            t->initializeThreadData(target);
            t->initializeThreadData(target2);
            t->wakeThread(target);
            t->AfterMutexUnlock(target, 0, mutex);
            TS_ASSERT(t->isEnabled(target));
        }

        void testBeforeCondWait(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            TS_ASSERT(t->isEnabled(target));
            t->BeforeCondWait(target, NULL, NULL);
            TS_ASSERT(t->isDisabled(target));
        }

        void testIsEnabledDisabled(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            TS_ASSERT(t->isEnabled(target));
            TS_ASSERT(!t->isDisabled(target));
            t->enable_map[target] = false;
            TS_ASSERT(t->isDisabled(target));
            TS_ASSERT(! t->isEnabled(target));
        }

        void testWaitPauseAndWakeThread(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            TS_ASSERT(getSemVal(t->wait_sem_map[target]) == 0);

            TS_ASSERT_THROWS_NOTHING(t->waitWakeThread(target));
            TS_ASSERT(getSemVal(t->wait_sem_map[target]) == 1);
            TS_ASSERT_THROWS_NOTHING(t->waitPauseThread(target));
            TS_ASSERT(getSemVal(t->wait_sem_map[target]) == 0);

            TS_ASSERT_THROWS_NOTHING(t->waitWakeThread(target));
            TS_ASSERT(getSemVal(t->wait_sem_map[target]) == 1);
            TS_ASSERT_THROWS_NOTHING(t->waitWakeThread(target));
            TS_ASSERT(getSemVal(t->wait_sem_map[target]) == 2);
            TS_ASSERT_THROWS_NOTHING(t->waitPauseThread(target));
            TS_ASSERT(getSemVal(t->wait_sem_map[target]) == 1);
            TS_ASSERT_THROWS_NOTHING(t->waitPauseThread(target));
            TS_ASSERT(getSemVal(t->wait_sem_map[target]) == 0);
        }

        void testSimulateCondWait(void) {
            pthread_t * thread3 = new pthread_t;
            pthread_t * thread4 = new pthread_t;
            thrID target = namer->getNewTID(thread);
            thrID target2 = namer->getNewTID(thread2);
            thrID target3 = namer->getNewTID(thread3);
            thrID target4 = namer->getNewTID(thread4);
            t->initializeThreadData(target);
            t->initializeThreadData(target2);
            t->initializeThreadData(target3);
            t->initializeThreadData(target4);
            t->addThreadWaitingOnLock(target2, mutex);
            t->disableThread(target2);
            t->addThreadReadyOnLock(target3, mutex);
            t->addThreadWaitingOnLock(target4, mutex2);
            t->disableThread(target4);
            t->disableThread(target);
            structSimulateCondWait * my_arg = new structSimulateCondWait(t,
                    mutex,
                    cond,
                    target);
            Originals::pthread_create(thread, 
                    NULL, 
                    threadSimulateCondWait, 
                    my_arg);
            Originals::usleep(50);
            TS_ASSERT(Originals::pthread_mutex_trylock(mutex) == 0);
            TS_ASSERT(Originals::pthread_mutex_unlock(mutex) == 0);
            TS_ASSERT(t->isEnabled(target2));
            TS_ASSERT(t->lock_ready_map[target2] == mutex);
            TS_ASSERT(t->lock_wait_map[target2] == NULL);
            TS_ASSERT(t->isEnabled(target3));
            TS_ASSERT(t->lock_ready_map[target3] == mutex);
            TS_ASSERT(t->lock_wait_map[target3] == NULL);
            TS_ASSERT(t->isDisabled(target4));
            TS_ASSERT(t->lock_ready_map[target4] == NULL);
            TS_ASSERT(t->lock_wait_map[target4] == mutex2);
            TS_ASSERT(t->cond_map[target] == cond);

            t->wakeThread(target);
            t->waitWakeThread(target);

            Originals::pthread_join(*thread, NULL);
            TS_ASSERT(t->cond_map[target] == NULL);
            delete thread3;
            delete thread4;
        }

        void testBeforeCondSignal(void) {
            thrID target = namer->getNewTID(thread);
            TS_ASSERT(! t->BeforeCondSignal(target, NULL));
        }

        void testMyMemoryRead(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            t->myMemoryRead(target, 1, NULL);
        }
        
        void testMyMemoryWrite(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            t->myMemoryWrite(target, 1, NULL);
        }

        void testBeforeCondBroadcast(void) {
            thrID target = namer->getNewTID(thread);
            TS_ASSERT(! t->BeforeCondBroadcast(target, NULL));
        }

        void testSimulateCondBroadcast(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            t->SimulateCondBroadcast(target, cond);
            TS_ASSERT(t->isEnabled(target));
        }

        void testSimulateCondSignal(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            t->SimulateCondSignal(target, cond);
            TS_ASSERT(t->isEnabled(target));
        }

        void testDetermineLockStatus(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            t->determineLockStatus(target, mutex);
            TS_ASSERT(t->isEnabled(target));
            TS_ASSERT(Originals::pthread_mutex_trylock(mutex) == 0);
            TS_ASSERT(Originals::pthread_mutex_unlock(mutex) == 0);
            TS_ASSERT(t->lock_ready_map[target] == mutex);
        }

        void testDetermineLockStatusB(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            TS_ASSERT(Originals::pthread_mutex_trylock(mutex) == 0);
            t->determineLockStatus(target, mutex);
            TS_ASSERT(Originals::pthread_mutex_unlock(mutex) == 0);
            TS_ASSERT(t->isDisabled(target));
            TS_ASSERT(t->lock_wait_map[target] == mutex);
        }


        void testWakeThreadWaitingOnCond(void) {
            thrID target = namer->getNewTID(thread);
            thrID target2 = namer->getNewTID(thread2);
            t->initializeThreadData(target);
            t->initializeThreadData(target2);
           
            t->glock();

            t->cond_map[target2] = cond;

            t->gunlock();
          
            structSimulateCondWait * my_arg = new structSimulateCondWait(t,
                    mutex,
                    cond,
                    target);
           
            Originals::pthread_create(thread, 
                    NULL, 
                    threadWakeThreadWaitingOnCond, 
                    my_arg);

            t->waitPauseThread(target2);
            
            t->glock();

            t->cond_map[target2] = NULL;

            t->gunlock();
            t->signalThreadReady(target2);
            
            Originals::pthread_join(*thread, NULL);

            TS_ASSERT(true);
            
        }

        void testWaitSignalThreadReadyA(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            structSimulateCondWait * my_arg = new structSimulateCondWait(t,
                    mutex,
                    cond,
                    target);
            Originals::pthread_create(thread, 
                    NULL, 
                    threadSignalThreadReadyA, 
                    my_arg);
            Originals::usleep(30);
            t->waitThreadReady(5, target);
            Originals::pthread_join(*thread, NULL);
        }

        void testWaitSignalThreadReadyB(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            structSimulateCondWait * my_arg = new structSimulateCondWait(t,
                    mutex,
                    cond,
                    target);
            Originals::pthread_create(thread, 
                    NULL, 
                    threadSignalThreadReadyB, 
                    my_arg);
            t->waitThreadReady(5, target);
            Originals::pthread_join(*thread, NULL);

        }

        void testIsSignalMissed(void) {
            pthread_t * thread3 = new pthread_t;
            thrID target = namer->getNewTID(thread);
            thrID target2 = namer->getNewTID(thread2);
            thrID target3 = namer->getNewTID(thread3);
            t->initializeThreadData(target);
            t->initializeThreadData(target2);
            t->initializeThreadData(target3);
            t->cond_map[target] = cond2;
            t->cond_map[target2] = cond;
            t->cond_map[target3] = cond;
            TS_ASSERT(! t->isSignalMissed(cond));
            TS_ASSERT(! t->isSignalMissed(cond2));
            t->cond_map[target] = cond;
            TS_ASSERT(! t->isSignalMissed(cond));
            TS_ASSERT(t->isSignalMissed(cond2));
            TS_ASSERT(t->isSignalMissed(NULL));
            delete thread3;
        }

        void testSelectThreadWaitingOnCond(void){
            pthread_t * thread3 = new pthread_t;
            thrID target = namer->getNewTID(thread);
            thrID target2 = namer->getNewTID(thread2);
            thrID target3 = namer->getNewTID(thread3);
            t->initializeThreadData(target);
            t->initializeThreadData(target2);
            t->initializeThreadData(target3);
            t->cond_map[target] = cond;
            t->cond_map[target2] = NULL;
            t->cond_map[target3] = cond2;
            thrID result = t->selectThreadWaitingOnCond(cond2);
            TS_ASSERT(result == target3);
            t->cond_map[target3] = cond;
            result = t->selectThreadWaitingOnCond(cond);
            TS_ASSERT( result == target || result == target3);
            t->cond_map[result] = NULL;
            result = t->selectThreadWaitingOnCond(cond);
            TS_ASSERT( result == target || result == target3);
            t->cond_map[result] = NULL;
            result = t->selectThreadWaitingOnCond(cond);
            TS_ASSERT(result == -1);
            delete thread3;
        }

        void testScheduleThreadWaitingOnCondA(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            t->cond_map[target] = NULL;
            TS_ASSERT_THROWS_ANYTHING(t->scheduleThreadWaitingOnCond(5, 
            target, cond, false));
        }

        void testPickNextSchedulingChoice(void) {
            thrID target = namer->getNewTID(thread);
            thrID target2 = namer->getNewTID(thread2);
            t->initializeThreadData(target);
            t->initializeThreadData(target2);
            t->disableThread(target);
            t->enableThread(target2);
            TS_ASSERT(target2 == t->pickNextSchedulingChoice());
            t->disableThread(target2);
            TS_ASSERT_THROWS_ANYTHING(t->pickNextSchedulingChoice());
        }

        void testBeforeCondTimedwait(void) {
            thrID target = namer->getNewTID(thread);
            thrID target2 = namer->getNewTID(thread2);
            t->initializeThreadData(target);
            t->initializeThreadData(target2);
            t->disableThread(target);
            bool result = t->BeforeCondTimedwait(target, cond, mutex, NULL);
            TS_ASSERT(false ==  result);
            TS_ASSERT(t->enable_map[target]);
        }

        void testSimulateCondTimedwait(void) {
            TS_WARN("make test for simulate cond timedwait");
        }

        void testLaunchThread(void) {
            thrID target = namer->getNewTID(thread);
            thrID target2 = namer->getNewTID(thread2);
            t->initializeThreadData(target);
            t->initializeThreadData(target2);
            t->cond_map[target2] = cond;
            t->launchThread(target2);
            t->waitPauseThread(target2);
            TS_ASSERT(t->was_signalled_map[target2] == false);
            t->cond_map[target2] = NULL;
            t->launchThread(target2);
            t->pauseThread(target2);
            TS_ASSERT(true);
            t->disableThread(target2);
            TS_ASSERT_THROWS_ANYTHING(t->launchThread(target2));
        }

        void testBeforeExit(void) {
            TS_ASSERT(false == t->BeforeExit(5, NULL));
        }

        void testSimulateExit(void) {
            TS_WARN("make test for simulate exit");
        }

        void testBeforeSleep(void) {
            TS_ASSERT(false == t->BeforeSleep(5, 0));
        }

        void testSimulateSleep(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            TS_ASSERT(0 == t->SimulateSleep(target, 1));
            TS_ASSERT(true);
        }

        void testBeforeUsleep(void) {
            TS_ASSERT(false == t->BeforeUsleep(5, 0));
        }

        void testSimulateUsleep(void) {
            thrID target = namer->getNewTID(thread);
            t->initializeThreadData(target);
            TS_ASSERT(0 == t->SimulateUsleep(target, 1));
            TS_ASSERT(true);
        }

};

void * threadSimulateCondWait(void * arg) {
    structSimulateCondWait * my_arg = 
        static_cast<structSimulateCondWait*>(arg);
    ExecutionTracker * t = my_arg->my_t;
    pthread_mutex_t * mutex = my_arg->my_mutex;
    pthread_cond_t * cond = my_arg->my_cond;
    thrID myself = my_arg->myself;
    t->SimulateCondWait(myself, cond, mutex);
}

void * threadSignalThreadReadyA(void * arg) {
    structSimulateCondWait * my_arg = 
        static_cast<structSimulateCondWait*>(arg);
    ExecutionTracker * t = my_arg->my_t;
    pthread_mutex_t * mutex = my_arg->my_mutex;
    pthread_cond_t * cond = my_arg->my_cond;
    thrID myself = my_arg->myself;
    t->signalThreadReady(myself);
}

void * threadSignalThreadReadyB(void * arg) {
    structSimulateCondWait * my_arg = 
        static_cast<structSimulateCondWait*>(arg);
    ExecutionTracker * t = my_arg->my_t;
    pthread_mutex_t * mutex = my_arg->my_mutex;
    pthread_cond_t * cond = my_arg->my_cond;
    thrID myself = my_arg->myself;
    Originals::usleep(30);
    t->signalThreadReady(myself);
}


void * threadWakeThreadWaitingOnCond(void * arg) {
    structSimulateCondWait * my_arg = 
        static_cast<structSimulateCondWait*>(arg);
    ExecutionTracker * t = my_arg->my_t;
    pthread_mutex_t * mutex = my_arg->my_mutex;
    pthread_cond_t * cond = my_arg->my_cond;
    thrID myself = my_arg->myself;
    t->wakeThreadWaitingOnCond(myself, cond, false);
} 
