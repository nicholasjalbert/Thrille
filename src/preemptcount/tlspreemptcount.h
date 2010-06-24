#ifndef _TLSPREEMPTCOUNTDATA_H_
#define _TLSPREEMPTCOUNTDATA_H_

#include "../thrille-core/ThreadInfo.h"

class TLSPreemptcountData : public ThreadInfo {
    public:
        TLSPreemptcountData() : ThreadInfo() { }
};

#endif

