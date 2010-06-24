/**
 * hybridtypes.h - Typedefs for hybrid race tracking 
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */

#ifndef _ACTIVETYPES_H_
#define _ACTIVETYPES_H_

#include <map>
#include <set>
#include <fstream>
#include <sys/time.h>
#include <vector>

using namespace std;

struct raceEvent {
    int iid;
    void * addr;
    bool isread;

    raceEvent(int theiid,
                void * theaddr,
                bool theisread) {
        iid = theiid;
        addr = theaddr;
        isread = theisread;
    }

    bool isRace(raceEvent * other) {
        if (addr != other->addr){
            return false;
        } else if (isread && other->isread) {
            return false;
        }
        return true;
    }

    bool operator==(const raceEvent & other) const {
        if (other.iid != iid)
            return false;
        if (other.addr != addr)
            return false;
        if (other.isread != isread)
            return false;
        return true;
    }

    bool operator!=(const raceEvent & other) const {
        return !(*this == other);
    }
    
    //TODO: fixme
    bool operator< (const raceEvent & c) const {
        if (iid != c.iid)
            return (iid < c.iid);
    }


    friend ostream& operator<<(ostream & output, const raceEvent & ev);
};



#endif
