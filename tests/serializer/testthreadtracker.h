// Tests src/serializer/executiontracker
// using cxxtest

#include "../cxxtest/cxxtest/TestSuite.h"
#include "../../src/serializer/executiontracker.h"
#include "../../src/serializer/threadtracker.h"


void * myproc(void * arg) {}

class ThreadTrackerTestSuite: public CxxTest::TestSuite {

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

        void testGetSetThreadId(void) {
            thrID tar1 = namer->getNewTID(thread);
            thrID tar2 = namer->getNewTID(thread2);
            Originals::pthread_create(thread, NULL, myproc, NULL); 
            Originals::pthread_create(thread2, NULL, myproc, NULL); 
            thrID result2 = namer->translateHandleToTID(thread2);
            thrID result1 = namer->translateHandleToTID(thread);
            TS_ASSERT(result1 == tar1);
            TS_ASSERT(result2 == tar2);
        }
        
};


