/**
* librandomschedule - implements a random (serialized) scheduler
* 
* Author - Nick Jalbert (jalbert@eecs.berkeley.edu)
*
* <Legal matter>
*/

#ifndef _LIBRANDOMSCHEDULE_H_
#define _LIBRANDOMSCHEDULE_H_

#include "../serializer/libserializer.h"
#include "randomtracker.h"

class RandomscheduleHandler : public SerializerHandler{
    private: 

    public:
        RandomscheduleHandler();
        virtual ~RandomscheduleHandler();
    
    protected:

        virtual ExecutionTracker * getNewExecutionTracker(thrID);

        friend class LibRandomscheduleTestSuite;
};

#endif
