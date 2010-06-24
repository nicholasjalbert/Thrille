#include "libblank.h"

// example additional interposition functions
// uncomment friend declaration in header
void myMemoryRead(int iid, void * addr) {
    // BlankHandler * myhandler = 
    //     dynamic_cast<BlankHandler *>(pHandler);
    // myhandler->myMemoryRead(iid, addr);
}

void myMemoryWrite(int iid, void * addr) {
    // BlankHandler * myhandler = 
    //     dynamic_cast<BlankHandler *>(pHandler);
    // myhandler->myMemoryWrite(iid, addr);
}

