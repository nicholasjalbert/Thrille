#include "libirace.h"

void myMemoryRead(int iid, void * addr) {
    IraceHandler * myhandler = 
        dynamic_cast<IraceHandler *>(pHandler);
    myhandler->memoryRead(iid, addr);
}

void myMemoryWrite(int iid, void * addr) {
    IraceHandler * myhandler = 
        dynamic_cast<IraceHandler *>(pHandler);
    myhandler->memoryWrite(iid, addr);
}

