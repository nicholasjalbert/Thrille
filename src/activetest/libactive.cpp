#include "libactive.h"

Handler * create_handler()
{
  return new ActiveHandler();
}

void ActiveHandler::prepTimedWait(timespec * target) {
    struct timeval now;
    gettimeofday(&now, NULL);
    target->tv_sec = now.tv_sec;
    target->tv_nsec = (now.tv_usec + 1000 * msTimeout) * 1000;
    if (target->tv_nsec >= 1000000000) {
        target->tv_sec += target->tv_nsec/1000000000;
        target->tv_nsec = target->tv_nsec % 1000000000;
    }
}

void ActiveHandler::myMemoryRead(int iid, void * addr) {
    if (iid == firstiid || iid == secondiid) {
        if (! raceDiscovered) {
            raceEvent r(iid, addr, true);
            Originals::pthread_mutex_lock(my_mutex);
            if (checkRace(r)) {
                raceDiscovered = true;
                Originals::pthread_cond_broadcast(my_cond);
                Originals::pthread_mutex_unlock(my_mutex);
                return;
            }
            addEvent(&r);
            struct timespec my_timeout;
            prepTimedWait(&my_timeout);
            pthread_cond_timedwait(my_cond, my_mutex, &my_timeout);
            removeEvent(&r);
            Originals::pthread_mutex_unlock(my_mutex);
        }
    }
}

void ActiveHandler::myMemoryWrite(int iid, void * addr) {
     if (iid == firstiid || iid == secondiid) {
        if (! raceDiscovered) {
            raceEvent r(iid, addr, false);
            Originals::pthread_mutex_lock(my_mutex);
            if (checkRace(r)) {
                raceDiscovered = true;
                Originals::pthread_cond_broadcast(my_cond);
                Originals::pthread_mutex_unlock(my_mutex);
                return;
            }
            addEvent(&r);
            struct timespec my_timeout;
            prepTimedWait(&my_timeout);
            pthread_cond_timedwait(my_cond, my_mutex, &my_timeout);
            removeEvent(&r);
            Originals::pthread_mutex_unlock(my_mutex);
        }
    }
}


bool ActiveHandler::checkRace(raceEvent r) {
    set<raceEvent *>::iterator itr;
    for (itr = eventSet.begin(); itr != eventSet.end(); itr++) {
        raceEvent * tmp = (*itr);
        if (r.isRace(tmp)) {
            return true;
        }
    }
    return false;
}

void ActiveHandler::addEvent(raceEvent * r) {
    eventSet.insert(r);
}

void ActiveHandler::AfterInitialize() {
    getTargetIIDs();
}


void ActiveHandler::getTargetIIDs() {
    ifstream iidfile;
    iidfile.open("thr-iids");
    iidfile >> firstiid;
    iidfile >> secondiid;
    iidfile.close();
    dbgPrint("Active Testing IIDs: %d and %d\n", firstiid, secondiid);
}



void ActiveHandler::removeEvent(raceEvent * r) {
    set<raceEvent *>::iterator itr =  eventSet.find(r);
    if (itr == eventSet.end()) {
        printf("ActiveError: 1");
        exit(5);
    }
    eventSet.erase(r);
}

void ActiveHandler::initHandler() {
    printf("Init\n"); 
    printf("lol\n");
    Originals::pthread_mutex_init(my_mutex, NULL);
    printf("1\n");
    Originals::pthread_cond_init(my_cond, NULL);
    printf("2\n");
    getTargetIIDs(); 
    isFirst = false;
    printf("After Init\n");
}



void myMemoryRead(int iid, void * addr) {
    ActiveHandler * myhandler = dynamic_cast<ActiveHandler *>(pHandler);
    myhandler->myMemoryRead(iid, addr);
}

void myMemoryWrite(int iid, void * addr) {
    ActiveHandler * myhandler = dynamic_cast<ActiveHandler *>(pHandler);
    myhandler->myMemoryWrite(iid, addr);
}

ostream& operator<<(ostream & output, const raceEvent & ev) {
    output << "RaceEvent(iid:" << ev.iid << ", addr:" << 
        (void *) ev.addr << ", isRead: " << ev.isread << ")" << 
        endl << flush; 
    return output;
}

