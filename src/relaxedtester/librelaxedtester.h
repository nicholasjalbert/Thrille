/**
* librelaxedtester- Implements a schedule playback serializer which 
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
*                    This thriller implementes a *strict* playback of the 
*                    relaxed schedule format--targetted for 
*                    sanity checks in simpl.py
* 
* Author - Nick Jalbert (jalbert@eecs.berkeley.edu)
*
* <Legal matter>
*/

#ifndef _LIBRELAXEDTESTER_H
#define _LIBRELAXEDTESTER_H

#include "../serializer/libserializer.h"
#include "testertracker.h"

class RelaxedtesterHandler : public SerializerHandler {
    private: 

    public:
        RelaxedtesterHandler();
        virtual ~RelaxedtesterHandler();

    protected:

        virtual ExecutionTracker * getNewExecutionTracker(thrID);
};

#endif
