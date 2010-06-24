#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

void myMemoryRead(int iid, void * addr) {
}

void myMemoryWrite(int iid, void * addr) {
}

extern "C" {
    void thrilleAssertC(int);
    void * thrilleMallocC(size_t);
    void thrilleCheckpointC();
    void thrilleSchedPointC();
}

void thrilleSchedPointC() {
    return;
}

void thrilleSchedPointCPP() {
    return;
}

void thrilleCheckpointC() {
    return;
}

void thrilleCheckpointCPP() {
    return;
}

void thrilleAssertC(int) {
    return;
}

void thrilleAssertCPP(bool) {}

void * thrilleMallocCPP(size_t s) { 
    return malloc(s);
}

void * thrilleMallocC(size_t s) { 
    return malloc(s);
}
