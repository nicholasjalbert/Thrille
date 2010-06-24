/**
* librelaxedserial - Implements a schedule playback serializer which 
*                    takes in a flexible schedule and will execute it
*                    (implements relaxed feasability function)
*
*                    Schedule form
*                    [Thread ID]:[Max Number of Times to Schedule]
*                    [Addr of Preemption]:[Times to pass before Preempt]
*                    [list of threads to signal sep by space and end with ##]
*                    ...
*                    ...
*                    ...
* 
* Author - Nick Jalbert (jalbert@eecs.berkeley.edu)
*
* <Legal matter>
*/

#ifndef _LIBRELAXEDSERIAL_H_
#define _LIBRELAXEDSERIAL_H_

#include "../serializer/libserializer.h"
#include "relaxedtracker.h"

class RelaxedserialHandler : public SerializerHandler {
    private: 

    public:
        RelaxedserialHandler();
        virtual ~RelaxedserialHandler();

    protected:

        virtual ExecutionTracker * getNewExecutionTracker(thrID);

        friend class LibRelaxedserialTestSuite;
};

#endif
