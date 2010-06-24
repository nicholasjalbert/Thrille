/**
* libpreemptcount - thriller that counts execution statistics   
* 
* Author - Nick Jalbert (jalbert@eecs.berkeley.edu)
*
* <Legal matter>
*/

#ifndef _LIBPREEMPTCOUNT_H_
#define _LIBPREEMPTCOUNT_H_

#include "../thrille-core/libpth.h"
#include "../serializer/libserializer.h"
#include "tlspreemptcount.h"
#include "preemptcounter.h"

class PreemptcountHandler : public SerializerHandler {
    private: 

    public:
        PreemptcountHandler();
        virtual ~PreemptcountHandler(); 

    protected:
        virtual ExecutionTracker * getNewExecutionTracker(thrID);
        friend class LibPreemptcountTestSuite;
};

#endif
