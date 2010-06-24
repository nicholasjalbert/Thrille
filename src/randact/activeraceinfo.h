/*
 * activeraceinfo - records a racing event
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _ACTIVERACEINFO_H_
#define _ACTIVERACEINFO_H_

#include "../serializer/serializertypes.h"

struct ActiveRaceInfo {
    thrID me;
    void * addr;
    bool iswrite;

    ActiveRaceInfo();

    ActiveRaceInfo(thrID _me, void * _addr, bool _iswrite); 

    virtual ~ActiveRaceInfo(); 

    virtual bool racesWith(ActiveRaceInfo);

};

#endif
