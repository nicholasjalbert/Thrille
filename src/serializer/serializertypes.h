/**
 * serializertypes.h - Typedefs for program serialization
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */

#ifndef _SERIALIZERTYPES_H_
#define _SERIALIZERTYPES_H_

#include "../thrille-core/libpth.h"
#include <sys/time.h>

#define UNKNOWN_THREAD -2
#define INVALID_THREAD -1
#define OPERATION_SUCCESSFUL 0
#define MAX_READ_SIZE 256
#define UNJOINED -1

/* These two definitions are for fair scheduling */
#define IS_YIELD true
#define IS_NOT_YIELD false

/* Determine if we're locking and unlockin in a cond_wait or normally */
#define IS_COND_WAIT true
#define IS_NOT_COND_WAIT false

/* For signal point info */
#define IS_BROADCAST true
#define IS_NOT_BROADCAST false

struct SchedPointInfo {
    void * ret_addr;
    void* memory_accessed;
    string type;
    thrID thread;
    map<thrID, bool> enabled;
    map<thrID, bool> fenmap;
    bool isYield;
    bool fromLog;

    SchedPointInfo(void * ret, string ty, thrID thr,
            map<thrID, bool> en, bool isy) {
        ret_addr = ret;
        type = ty;
        thread = thr;
        enabled = en;
        isYield = isy;
        memory_accessed = NULL;
    }
};


struct SignalPointInfo {
    void * ret_addr;
    bool isBroadcast;
    thrID thread;
    pthread_cond_t * cond;
    map<thrID, pthread_cond_t *> cond_map;

    SignalPointInfo(void * ret, bool broad,
            thrID thr, pthread_cond_t * cnd,
            map<thrID, pthread_cond_t *> cnd_map) {
        ret_addr = ret;
        isBroadcast = broad;
        thread = thr;
        cond = cnd;
        cond_map = cnd_map;
    }
};


class DecisionInfo {
    protected:
        thrID chosen;
        thrID caller;
        void * idaddr;

    public:
        DecisionInfo(thrID, thrID, void *); 
        virtual ~DecisionInfo();
        virtual void printToFile(FILE *) = 0;
        void printToStdout();
        thrID getChosen();
        void * getID();
};

class ScheduleDecisionInfo: public DecisionInfo {
    private:
        string description;
        vector<thrID> enabled;

    public:
        void* memory_accessed;
        ScheduleDecisionInfo(thrID, thrID, string, void *); 
        virtual void printToFile(FILE *);
        void addEnabledThread(thrID);
        string changeSpacesToUnderscores(string);
};

class SignalDecisionInfo: public DecisionInfo {
    private:
        void * cond;
        vector<thrID> oncond;

    public:
        SignalDecisionInfo(thrID, thrID, void *, void *); 
        virtual void printToFile(FILE *);
        void addThreadOnCond(thrID);
};







#endif
