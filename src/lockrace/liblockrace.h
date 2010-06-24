/**
* liblockrace - race detection that also reports lock contentions
* 
* Author - Nick Jalbert (nickjalbert@gmail.com)
*
* <Legal matter>
*/

#ifndef _LIBLOCKRACE_H_
#define _LIBLOCKRACE_H_

#include "../racer/libracer.h"
#include "modtrack.h"

class LockraceHandler : public RacerHandler{
    protected:

    public:
        LockraceHandler();
        virtual ~LockraceHandler();
 
        friend class LibLockraceTestSuite;
};

#endif
