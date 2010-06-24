// Tests src/serializer/logger
// using cxxtest

#include "../cxxtest/cxxtest/TestSuite.h"
#include "../../src/serializer/logger.h"

class LoggerTestSuite : public CxxTest::TestSuite {

    public:

        void testSimple() {
            TS_ASSERT(true);
        }

};
