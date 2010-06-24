// Tests src/activetest/libactive.h
// using cxxtest

#include "../cxxtest/cxxtest/TestSuite.h"
#include "../../src/activetest/libactive.h"

class LibactiveTestSuite : public CxxTest::TestSuite {

    public:

        void testSimple() {
            TS_ASSERT(true);
        }

};
