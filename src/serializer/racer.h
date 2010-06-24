#include "serializertypes.h"

class Racer {
    protected:
        thrID main_thr;

    public:
        Racer(thrID main);

        virtual ~Racer();

        virtual void ThreadStart(thrID); 
        virtual void ThreadFinish(thrID); 
        virtual void BeforeCreate(thrID, thrID); 
        virtual void AfterJoin(thrID, thrID);
        virtual void AfterMutexLock(thrID, pthread_mutex_t *);
        virtual void BeforeMutexUnlock(thrID, pthread_mutex_t *);
        virtual void BeforeCondSignal(thrID, pthread_cond_t *);
        virtual void BeforeCondBroadcast(thrID, pthread_cond_t *);
        virtual void AfterCondWait(thrID, pthread_cond_t *);
        virtual void AfterCondTimedwait(thrID, int, pthread_cond_t *); 
        virtual void memoryRead(thrID, void *, void *);
        virtual void memoryWrite(thrID, void *, void *);
};


