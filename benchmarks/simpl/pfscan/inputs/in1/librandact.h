/**
* librandact - implements random active testing on top of serialization
* for replayability
* 
* Author -  Nick Jalbert (jalbert@eecs.berkeley.edu)
*
* <Legal matter>
*/

#ifndef _LIBRANDACT_H_
#define _LIBRANDACT_H_

#include "../serializer/libserializer.h"
#include "randomlock.h"
#include "randomdata.h"
#include "tlsrandact.h"

class RandactHandler : public SerializerHandler{
    private: 

    public:
        RandactHandler(); 
        virtual ~RandactHandler();

    protected:
        virtual ExecutionTracker * getNewExecutionTracker(thrID);
        virtual bool isLockRace();

        friend class LibRandactTestSuite;
};

#endif
