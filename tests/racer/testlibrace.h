// Tests src/racedetect/librace.h
// using cxxtest

#include "../cxxtest/cxxtest/TestSuite.h"
#include "../../src/racedetect/librace.h"

class LibraceTestSuite : public CxxTest::TestSuite {

    public:

        void testSimple() {
            TS_ASSERT(true);
        }

};
