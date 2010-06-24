/**
* libchessserial - implements a schedule playback serializer which will follow
* a schedule until it ends and then perform non-preemptive fair execution
* 
* Author - Nick Jalbert (jalbert@eecs.berkeley.edu)
*
* <Legal matter>
*/

#ifndef _LIBCHESSSERIAL_H_
#define _LIBCHESSSERIAL_H_

#include "../serializer/libserializer.h"
#include "chesstracker.h"

class ChessserialHandler : public SerializerHandler {
    private: 

    public:
        ChessserialHandler();
        virtual ~ChessserialHandler();

    protected:

        virtual ExecutionTracker * getNewExecutionTracker(thrID);

        friend class LibChessserialTestSuite;
};

#endif
