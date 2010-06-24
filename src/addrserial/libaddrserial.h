/**
* libaddrserial - Implements a schedule playback serializer which 
*                    takes in a flexible schedule and will execute it
*                    (implements addr relaxation only)
*
*                    Schedule form
*                    [Thread ID]:[Max Number of Times to Schedule]
*                    [Addr of Preemption]:[Times to pass before Preempt]
*                    [list of threads to signal sep by space and end with ##]
*                    ...
*                    ...
*                    ...
*
*                    In this schedule, [Max Number of Times to Schedule] is 
*                    ignored.  Instead, we will schedule [Thread ID] until it
*                    is disabled, or we hit [Addr of Preemption] the specified
*                    number of times, at which point we will switch
* 
* Author - Nick Jalbert (jalbert@eecs.berkeley.edu)
*
* <Legal matter>
*/

#ifndef _LIBADDRSERIAL_H_
#define _LIBADDRSERIAL_H_

#include "../serializer/libserializer.h"
#include "addrtracker.h"

class AddrserialHandler : public SerializerHandler {
    private: 

    public:
        AddrserialHandler();
        virtual ~AddrserialHandler();

    protected:

        virtual ExecutionTracker * getNewExecutionTracker(thrID);

        friend class LibAddrserialTestSuite;
};

#endif
