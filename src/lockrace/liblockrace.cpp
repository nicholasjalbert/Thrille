#include "liblockrace.h"

LockraceHandler::LockraceHandler() : RacerHandler() {
    printf("Starting Lockrace Thriller...\n");
    unsigned int wr = eb->getWrProfileSize();
    unsigned int rd = eb->getRdProfileSize();
    delete eb;
    eb = new ModifiedRaceTracker(wr, rd);
}

LockraceHandler::~LockraceHandler() {
    printf("Ending Lockrace Thriller...\n");
}


