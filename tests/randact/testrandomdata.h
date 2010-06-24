// Tests src/randomschedule/librandomschedule.h
// using cxxtest

#include "../cxxtest/cxxtest/TestSuite.h"
#include "../../src/randact/librandact.h"

class RandomDataTesterTestSuite: public CxxTest::TestSuite {

    public:
        thrID target;
        thrID target2;
        thrID target3;
        RandomDataTester * dat;
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
            dat = new RandomDataTester(target, true);
            dat->initializeThreadData(target2);
            dat->initializeThreadData(target3);
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
            delete dat;
            delete namer;
            flag = true;
        }

        void testReenableThreadIfLegal() {
            TS_ASSERT_THROWS_ANYTHING(dat->reenableThreadIfLegal(target));
            dat->active_testing_paused[target] = true;
            dat->enable_map[target] = false;
            dat->reenableThreadIfLegal(target);
            TS_ASSERT(dat->enable_map[target]);
            TS_ASSERT(dat->active_testing_paused[target] == false);
        }

        void testHandleMyMemoryReadWrite() {
            dat->raceFound = true;
            dat->handleMyMemoryRead(target, 0, (void *) 0x55);
            dat->handleMyMemoryRead(target, 0, (void *) 0x55);
            dat->raceFound = false;
        }

        void testHandleMyMemoryReadWriteB() {
            dat->handleMyMemoryRead(target, 0, (void *) 0x55);
            TS_ASSERT(dat->active_testing_paused[target]);
            TS_ASSERT(dat->enable_map[target] == false);
            dat->handleMyMemoryWrite(target2, 0, (void *) 0x55);
            TS_ASSERT(dat->raceFound = true);
            TS_ASSERT(dat->active_testing_paused[target2] == false);
            TS_ASSERT(dat->active_testing_paused[target] == false);
            TS_ASSERT(dat->enable_map[target2]);
            TS_ASSERT(dat->enable_map[target]);
        }
        
        void testHandleMyMemoryReadWriteC() {
            dat->handleMyMemoryRead(target, 0, (void *) 0x55);
            TS_ASSERT(dat->active_testing_paused[target]);
            TS_ASSERT(dat->enable_map[target] == false);
            dat->handleMyMemoryRead(target2, 0, (void *) 0x55);
            TS_ASSERT(dat->raceFound == false);
            TS_ASSERT(dat->active_testing_paused[target2] );
            TS_ASSERT(dat->active_testing_paused[target] );
            TS_ASSERT(dat->enable_map[target2] == false);
            TS_ASSERT(dat->enable_map[target] == false);
        }


};
