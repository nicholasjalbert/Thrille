#include "activeraceinfo.h"

bool ActiveRaceInfo::racesWith(ActiveRaceInfo other) {
    if (me == other.me) {
        safe_assert(false);
    }
    if (addr != other.addr) {
        return false;
    }
    if (iswrite || other.iswrite) {
        return true;
    }
    return false;
}

ActiveRaceInfo::ActiveRaceInfo() {
    me = -1;
    addr = NULL;
    iswrite = false;
}

ActiveRaceInfo::ActiveRaceInfo(thrID _me, void * _addr, bool _iswrite) {
    me = _me;
    addr = _addr;
    iswrite = _iswrite;
}

ActiveRaceInfo::~ActiveRaceInfo() {

}


