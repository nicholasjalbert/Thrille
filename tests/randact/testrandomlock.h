// Tests src/randomschedule/librandomschedule.h
// using cxxtest

#include "../cxxtest/cxxtest/TestSuite.h"
#include "../../src/randact/librandact.h"

class RandomLockTesterTestSuite: public CxxTest::TestSuite {

    public:
        thrID target;
        thrID target2;
        thrID target3;
        RandomLockTester * lat;
        ThreadTracker * namer;
        pthread_t * thread;
        pthread_t * thread2;
        pthread_t * thread3;
        pthread_mutex_t * mutex;
        pthread_mutex_t * mutex2;
        pthread_cond_t * cond;
        pthread_cond_t * cond2;
        bool flag;

        void setUp() {
            namer = new ThreadTracker();
            target = namer->getNewTID(thread);
            target2 = namer->getNewTID(thread2);
            target3 = namer->getNewTID(thread3);
            lat = new RandomLockTester(target);
            lat->initializeThreadData(target2);
            lat->initializeThreadData(target3);
            lat->target1 = (void *) 100;
            lat->target2 = (void *) 200;
            flag = true;
            thread = new pthread_t;
            thread2 = new pthread_t;
            thread3 = new pthread_t;
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
            delete thread3;
            delete thread2;
            delete thread;
            delete lat;
            delete namer;
            flag = true;
        }


        void testHandleBeforeMutexLock() {
            lat->raceFound = true;
            lat->handleBeforeMutexLock(target, mutex, (void *) 100);
            TS_ASSERT(lat->active_testing_paused[target] == false);
        }
        
        void testHandleBeforeMutexLockB() {
            lat->handleBeforeMutexLock(target, mutex, (void *) 100);
            lat->handleBeforeMutexLock(target2, mutex2, (void *) 200);
            TS_ASSERT(lat->raceFound == false);
            TS_ASSERT(lat->active_testing_paused[target]);
            TS_ASSERT(lat->active_testing_paused[target2]);
            TS_ASSERT(lat->enable_map[target] == false);
            TS_ASSERT(lat->enable_map[target2] == false);
        }
        
        void testHandleBeforeMutexLockC() {
            lat->handleBeforeMutexLock(target, mutex, (void *) 100);
            lat->handleBeforeMutexLock(target2, mutex, (void *) 200);
            TS_ASSERT(lat->raceFound == true);
            TS_ASSERT(lat->enable_map[target]);
            TS_ASSERT(lat->enable_map[target2]);
            TS_ASSERT(lat->active_testing_paused[target] == false);
            TS_ASSERT(lat->active_testing_paused[target2] == false);
        }

        void testHandleAfterMutexLock() {
            lat->active_testing_paused[target2] = true;
            lat->handleAfterMutexUnlock(target, mutex);
            TS_ASSERT(lat->enable_map[target2] == false);
            TS_ASSERT(lat->enable_map[target]);
        }

        void testReenableThreadIfLegal() {
            lat->active_testing_paused[target2] = true;
            lat->enable_map[target2] = false;
            ActiveRaceInfo tmp (target2, mutex, (void *) 100);
            lat->active_testing_info[target2] = tmp;
            TS_ASSERT(lat->reenableThreadIfLegal(target2));
            TS_ASSERT(lat->active_testing_paused[target2] == false);

        } 

        void testReenableThreadIfLegalB() {
            Originals::pthread_mutex_lock(mutex);
            lat->active_testing_paused[target2] = true;
            lat->enable_map[target2] = false;
            ActiveRaceInfo tmp (target2, mutex, (void *) 100);
            lat->active_testing_info[target2] = tmp;
            TS_ASSERT(lat->reenableThreadIfLegal(target2) == false);
            TS_ASSERT(lat->active_testing_paused[target2] == false);
        } 

};
