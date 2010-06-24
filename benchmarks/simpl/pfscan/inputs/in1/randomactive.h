/*
 * randomactive - integrates random active testing with the serializer work
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _RANDOMACTIVE_H_
#define _RANDOMACTIVE_H_

#include <fstream>
#include <sstream>
#include <vector>
#include "../randomschedule/randomtracker.h"
#include "activeraceinfo.h"

class RandomActiveTester: public RandomTracker {
    protected:
        map<thrID, bool> active_testing_paused;
        map<thrID, ActiveRaceInfo> active_testing_info;

        bool raceFound;
        string targetfile;
        virtual thrID pickNextSchedulingChoice();
        virtual vector<thrID> isRacing(ActiveRaceInfo);
        virtual bool reenableThreadIfLegal(thrID);
        virtual void livelockPrevention();
        virtual void enableAllActiveTestingPaused();
        virtual void enableSpecificActiveTestingPaused(vector<thrID>);
        virtual string rotateInputFile(ifstream *, ostringstream *);
    
    public:

        RandomActiveTester(thrID);
        virtual ~RandomActiveTester(); 

        friend class RandomActiveTesterTestSuite;

};

#endif
