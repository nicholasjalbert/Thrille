#ifndef _TLSIRACEDATA_H_
#define _TLSIRACEDATA_H_

#include "../thrille-core/ThreadInfo.h"
#include "iracetypes.h"

class TLSIraceData : public ThreadInfo {
    
    public:
        thrID parent;
        thrID me;
        TLSIraceData(thrID the_parent, thrID the_me) : ThreadInfo() { 
            parent = the_parent;
            me = the_me;
        }
};

#endif

