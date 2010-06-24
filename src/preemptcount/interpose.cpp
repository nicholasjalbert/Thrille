#include "libpreemptcount.h"

// example additional interposition functions
// uncomment friend declaration in header
void myMemoryRead(int iid, void * addr) {
    // PreemptcountHandler * myhandler = 
    //     dynamic_cast<PreemptcountHandler *>(pHandler);
    // myhandler->myMemoryRead(iid, addr);
}

void myMemoryWrite(int iid, void * addr) {
    // PreemptcountHandler * myhandler = 
    //     dynamic_cast<PreemptcountHandler *>(pHandler);
    // myhandler->myMemoryWrite(iid, addr);
}

