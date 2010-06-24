// Tests src/randomschedule/librandomschedule.h
// using cxxtest

#include "../cxxtest/cxxtest/TestSuite.h"
#include "../../src/randact/librandact.h"

class RandomActiveTesterTestSuite: public CxxTest::TestSuite {

    public:
        thrID target;
        thrID target2;
        thrID target3;
        RandomActiveTester * rat;
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
            rat = new RandomActiveTester(target);
            rat->initializeThreadData(target2);
            rat->initializeThreadData(target3);
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
            delete rat;
            delete namer;
            flag = true;
        }

        void testIsRacing(void) {
            ActiveRaceInfo tmp1 (target, (void *) 0x55, true);
            ActiveRaceInfo tmp2 (target, (void *) 0x55, true);
            rat->active_testing_paused[target] = true;
            rat->active_testing_info[target] = tmp1;
            TS_ASSERT_THROWS_ANYTHING(rat->isRacing(tmp2));
            ActiveRaceInfo tmp3 (target2, (void *) 0x55, false);
            TS_ASSERT(rat->isRacing(tmp3));
            rat->active_testing_info[target] = tmp3;
            ActiveRaceInfo tmp4 (target3, (void *) 0x55, false);
            TS_ASSERT(! rat->isRacing(tmp4));
        }

        void testEnableAllActiveTestingPaused() {
            rat->active_testing_paused[target] = true;
            rat->active_testing_paused[target2] = false;
            rat->active_testing_paused[target3] = true;
            rat->enableAllActiveTestingPaused();
            TS_ASSERT(rat->active_testing_paused[target] == false);
            TS_ASSERT(rat->active_testing_paused[target2] == false);
            TS_ASSERT(rat->active_testing_paused[target3] == false);
            TS_ASSERT(rat->enable_map[target] == false);
            TS_ASSERT(rat->enable_map[target2]);
            TS_ASSERT(rat->enable_map[target3] == false);
        }

        void testLivelockPrevention () {
            rat->active_testing_paused[target] = true;
            while (true) {
                rat->livelockPrevention();
                if (!rat->active_testing_paused[target]) {
                    break;
                }
            }
            TS_ASSERT(true);
        }

        void testPickNextSchedulingChoice() {
            rat->disableThread(target2);
            rat->disableThread(target3);
            TS_ASSERT(rat->pickNextSchedulingChoice() == target);
            rat->disableThread(target);
            TS_ASSERT_THROWS_ANYTHING(rat->pickNextSchedulingChoice());
        }

        void testReenableThreadIfLegal() {
            TS_ASSERT_THROWS_ANYTHING(rat->reenableThreadIfLegal(target));
            rat->active_testing_paused[target] = true;
            rat->reenableThreadIfLegal(target);
            TS_ASSERT(! rat->enable_map[target]);
            TS_ASSERT(! rat->active_testing_paused[target]);
        }


};
