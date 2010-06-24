/**
 * fairschedule - exposes an interface that allows thrillers
 * to use the fair scheduler algorithm
 *
 * Intended use - choose an enabled thread from the fair enabled map,
 * then inform the fair shceduler of your choice
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _FAIRSCHEDULE_H_
#define _FAIRSCHEDULE_H_

#include <set>
#include <vector>
#include <string>
#include "../serializer/serializertypes.h"

class FairScheduler {
    private:
        map<thrID, set<thrID> > fair_priority;
        map<thrID, set<thrID> > fair_enabled;
        map<thrID, set<thrID> > fair_disabled;
        map<thrID, set<thrID> > fair_scheduled;
        vector<thrID> schedule_record;
        set<thrID> prev_ES;
        bool justYield;

    public:
        FairScheduler();
        virtual ~FairScheduler();

        virtual bool isFairEnabled(thrID,
                map<thrID, bool>);
        virtual void printThisSet(set<thrID>);
        virtual void printThisMap(string, map<thrID, set<thrID> >);
        virtual void printThisVector(vector<thrID>);


        //goes after the call to schedule AND pause in a yield
        virtual void lastWasYield();


        //call these two in pairs 
        virtual map<thrID, bool> getFairEnabledMap(map<thrID, bool>);
        virtual void fairChosen(thrID);
        //


    protected:
        virtual thrID getLastScheduled();
        virtual void updateDataStructures(map<thrID, bool>);
};


#endif

