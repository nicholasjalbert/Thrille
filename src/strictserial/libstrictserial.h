/**
* libstrictserial - implements a schedule playback serializer which will fail
*                   when it can't follow a give schedule anymore (e.g. the 
*                   recorded schedule has been replayed but the program has
*                   not completed yet)
* 
* Author - Nick Jalbert (jalbert@eecs.berkeley.edu)
*
* <Legal matter>
*/

#ifndef _LIBSTRICTSERIAL_H_
#define _LIBSTRICTSERIAL_H_

#include "../serializer/libserializer.h"
#include "stricttracker.h"

class StrictserialHandler : public SerializerHandler {
    private: 

    public:
        StrictserialHandler();
        virtual ~StrictserialHandler();

    protected:

        virtual ExecutionTracker * getNewExecutionTracker(thrID);

        friend class LibStrictserialTestSuite;
};

#endif
