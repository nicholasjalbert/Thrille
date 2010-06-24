// Tests src/randomschedule/librandomschedule.h
// using cxxtest

#include "../cxxtest/cxxtest/TestSuite.h"
#include "../../src/randomschedule/librandomschedule.h"

class RandomTrackerTestSuite: public CxxTest::TestSuite {

    public:
        thrID target;
        thrID target2;
        thrID target3;
        RandomTracker * rt;
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
            rt = new RandomTracker(target);
            rt->initializeThreadData(target2);
            rt->initializeThreadData(target3);
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
            delete rt;
            delete namer;
            flag = true;
        }

        void testPickNextSchedulingChoice() {
            rt->disableThread(target);
            rt->disableThread(target2);
            TS_ASSERT(rt->pickNextSchedulingChoice() == target3);
            rt->disableThread(target3);
            TS_ASSERT_THROWS_ANYTHING(rt->pickNextSchedulingChoice());

        }

        void testSelectThreadWaitingOnCond() {
            rt->cond_map[target] = cond;
            rt->cond_map[target2] = cond2;
            rt->cond_map[target3] = cond;
            thrID choice = rt->selectThreadWaitingOnCond(cond2);
            TS_ASSERT(choice == target2);
            rt->cond_map[target2] = cond;
            TS_ASSERT_THROWS_ANYTHING(rt->selectThreadWaitingOnCond(cond2));

        }

        void testChooseRandomThread() {
            bool arr[3];
            arr[0] = false;
            arr[1] = false;
            arr[2] = false;
            thrID choice;
            for (int i = 0; i < 100; i++) {
                choice = rt->chooseRandomThread(rt->enable_map.begin(),
                        rt->enable_map.end());
                if (choice == target) {
                    arr[0] = true;
                    if (arr[0] && arr[1] && arr[2]) {
                        return;
                    }
                } else if (choice == target2) {
                    arr[1] = true;
                    if (arr[0] && arr[1] && arr[2]) {
                        return;
                    }
                } else if (choice == target3) {
                    arr[2] = true;
                    if (arr[0] && arr[1] && arr[2]) {
                        return;
                    }
                }
            }
            TS_ASSERT(false);
        }

};
