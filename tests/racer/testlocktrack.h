// Tests src/racedetect/locktrack.h
// using cxxtest

#include "../cxxtest/cxxtest/TestSuite.h"
#include "../../src/racedetect/locktrack.h"

class LocktrackTestSuite : public CxxTest::TestSuite {

    public:

        void testSimple() {
            TS_ASSERT(true);
        }

};
