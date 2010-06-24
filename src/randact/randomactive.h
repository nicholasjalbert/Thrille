/*
 * randomactive - integrates random active testing with the serializer work
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _RANDOMACTIVE_H_
#define _RANDOMACTIVE_H_

#include "../randomschedule/randomtracker.h"
#include "activeraceinfo.h"

class RandomActiveTester: public RandomTracker {
    protected:
        void * target1;
        void * target2;
        
        void setTestingTargets();

        map<thrID, bool> active_testing_paused;
        map<thrID, ActiveRaceInfo> active_testing_info;

        bool raceFound;
        string targetfile;
        int sparsifyNum;
        int sparsifyDenom;
        void setMemoryAsSchedulingPoint(char *);
        virtual thrID pickNextSchedulingChoice(SchedPointInfo *);
        virtual vector<thrID> isRacing(ActiveRaceInfo);
        virtual bool reenableThreadIfLegal(thrID);
        virtual void livelockPrevention();
        virtual void enableAllActiveTestingPaused();
        virtual void enableSpecificActiveTestingPaused(vector<thrID>);
        void rotateInputFile(FILE *, char *, char *, int);
        void setTargets(FILE *, char *, int);
        void rewriteTargetFile(FILE *, char *);
    public:

        RandomActiveTester(thrID);
        virtual ~RandomActiveTester(); 

        friend class RandomActiveTesterTestSuite;

};

#endif
