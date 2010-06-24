/**
 * relaxedlogger - reads the relaxed schedules (outputs normal schedules)
 * 
 * Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
 *
 * <Legal matter>
 */


#ifndef _RELAXEDLOGGER_H_
#define _RELAXEDLOGGER_H_

#include "../serializer/logger.h"

struct RelaxedSchedStruct {
    thrID current;
    int max;
    void * addr;
    int pass;
    vector <thrID> signals;

    RelaxedSchedStruct(thrID curr, int m, void * ad, int p) {
        current = curr;
        max = m;
        addr = ad;
        pass = p;
    }

    void print() {
        printf("RelaxedSchedStruct(");
        printf("Thread:%d, ", current);
        printf("Max:%d, ", max);
        printf("Addr:%p, ", addr);
        printf("Pass:%d, ", pass);
        printf("Signals:");
        vector<thrID>::iterator itr;
        for (itr = signals.begin(); itr != signals.end(); itr++) {
            printf("%d,", (*itr));
        }
        printf(")\n");
    }
};


class RelaxedLogger: public Logger {
    public:
        RelaxedLogger(string, string);
        virtual ~RelaxedLogger();
        virtual RelaxedSchedStruct regenerateSched();
        virtual thrID getNextSignalChoice();


    protected:

        vector<thrID>::iterator tts_itr;
        vector<thrID> threadsToSignal;
        void reconstituteRelaxedSchedule();
        vector <RelaxedSchedStruct *> relaxed_schedule;

        RelaxedSchedStruct getSchedulingStruct();
        void resetThreadsToSignal(RelaxedSchedStruct);


};


#endif

