/**
* logger - module that does the logging of an execution 
* 
* Author - Nick Jalbert (jalbert@eecs.berkeley.edu) 
*
* <Legal matter>
*/

#ifndef _LOGGER_H_
#define _LOGGER_H_ 

#include "serializertypes.h"
#include "../Mersenne-1.1/MersenneTwister.h"

class Logger {
    private:
        vector<DecisionInfo *> schedule;
        vector<DecisionInfo *> read_schedule;
        bool print;
        set<void *> addressToSchedule;
        FILE * schedulefile;
        bool scheduleAll;
        string lastID;
        MTRand * myrand;
        
        string logfile;
        string _infile;
        string _outfile;
    protected:
        bool isReadfileEnd(char *, int);
        void checkToken(char *, char *, int);
        thrID parseNextReadfileLineAsThrID();
        void clearLinesFromReadfile(int);
        char * trimWhitespace(char *);
        void setWhitespaceToColon(char *, int);
        string parseNextReadfileLineAsString();
        void * parseNextReadfileLineAsVoidStar();
        char * parseNextReadfileLineAsCharStar();

        void closeReadfile();
        int schedulePointsToSkip;
        int pointsLeftToSkip;

        void setPrint();
        void allocateObjects();
        virtual thrID getNextChoice();

        void reconstituteSchedule();
        void setSchedulableAddresses();
        FILE * readfile;

        void deallocateSchedule();
        void deallocateReadSchedule();
    public:

        Logger(string, string);

        void printCurrentSchedule();
        void printCurrentScheduleToFile(FILE *);

        void * getAddress(char *);
        virtual unsigned long getLogHash();
        void programAssertFail(thrID, void *);
        void programDeadlock();
        virtual void seedGenerator(unsigned long);
        virtual int getRandomNumber(int);
        virtual int getRandomNumber();
        virtual int getBadRandomNumber();
        virtual void sparsifyMemorySchedulingPoints(int, int);
        virtual ~Logger();
        virtual int numAddrToSchedule();
        virtual void scheduleAtAllMemory();
        virtual void scheduleAtNoMemory();
        virtual string getLastID();

        virtual void flushLogs();

        virtual void assertFail(string, string, int);

        
        bool tryToSchedule();

        virtual void scheduling(thrID, SchedPointInfo *);
        virtual void fairScheduling(thrID, SchedPointInfo *);
        virtual void signalScheduling(thrID, SignalPointInfo *);
        
        virtual void ThreadStart(thrID, void *);
        virtual void ThreadFinish(thrID, void *);
        virtual bool getPrint();
        void cleanClose();
        virtual void recordAddrSchedulingPoints();
        void addAddressToSchedule(void*);
        virtual bool isSchedulingPoint(void *);

        virtual void BeforeBarrierInit(thrID);
        virtual void BeforeBarrierWait(thrID);

        virtual void segfault(thrID);

        virtual void BeforeCreate(thrID, thrID);
        virtual void AfterCreate(thrID);
        virtual void pauseThread(thrID);
        virtual void BeforeSchedPoint(thrID);
        virtual void BeforeJoin(thrID, thrID);
        virtual void BeforeMutexLock(thrID, void *, pthread_mutex_t *, bool);
        virtual void SimulateMutexLock(thrID, bool);
        
        virtual void BeforeMutexTrylock(thrID); 
        virtual void SimulateMutexTrylock(thrID);

        virtual void AfterMutexUnlock(thrID, bool);
        virtual void waitPauseThread(thrID);
        virtual void waitWakeThread(thrID);
        virtual void BeforeCondWait(thrID);
        virtual void SimulateCondWait(thrID);
        virtual void BeforeSemWait(thrID);
        virtual void SimulateSemWait(thrID);
        virtual void SimulateSemPost(thrID);
        virtual void myMemoryRead(thrID, void *);
        virtual void myMemoryWrite(thrID, void *);
        virtual void SignalMissed(thrID, bool);
        virtual void BeforeCondTimedwait(thrID);
        virtual void SimulateCondTimedwait(thrID);
        virtual void BeforeExit(thrID);
        virtual void SimulateExit(thrID);
        virtual void myUsleep(thrID);
        virtual void mySleep(thrID);
        virtual void timedwaitNoTimeout(thrID);
        virtual void timedwaitTimeout(thrID);
        virtual thrID getNextSignalChoice();
        virtual thrID getNextScheduleChoice(SchedPointInfo *);

        friend void myMemoryWrite(int, void *);
        friend void myMemoryRead(int, void *);
};

#endif
