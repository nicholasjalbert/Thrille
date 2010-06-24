// Tests src/serializer/libserializer   
// using cxxtest

#include "../cxxtest/cxxtest/TestSuite.h"
#include "../../src/serializer/libserializer.h"

class LibSerializerTestSuite : public CxxTest::TestSuite {

    public:

        void testSimple() {
            TS_ASSERT(true);
        }


};
