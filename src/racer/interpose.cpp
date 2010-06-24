#include "libracer.h"

void myMemoryRead(int iid, void * addr) {
    RacerHandler * myhandler = dynamic_cast<RacerHandler *>(pHandler);
    void * my_id = __builtin_return_address(0);
    myhandler->myMemoryRead(my_id, addr);
}

void myMemoryWrite(int iid, void * addr) {
    RacerHandler * myhandler = dynamic_cast<RacerHandler *>(pHandler);
    void * my_id = __builtin_return_address(0);
    myhandler->myMemoryWrite(my_id, addr);
}

