/**
* libdfailserial - Implements a schedule playback serializer which 
*                    takes in a flexible schedule and will execute it
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
*                    NOTE:  this differs from libaddrserial in that if a thread
*                    becomes unexpectedly disabled before we expect it to, 
*                    we will FAIL.
* 
* Author - Nick Jalbert (jalbert@eecs.berkeley.edu)
*
* <Legal matter>
*/

#ifndef _LIBDFAILSERIAL_H_
#define _LIBDFAILSERIAL_H_

#include "../addrserial/libaddrserial.h"
#include "dfailtracker.h"

class DfailserialHandler : public AddrserialHandler {
    private: 

    public:
        DfailserialHandler();
        virtual ~DfailserialHandler();

    protected:

        virtual ExecutionTracker * getNewExecutionTracker(thrID);

        friend class LibDfailserialTestSuite;
};

#endif
