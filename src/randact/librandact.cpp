#include "librandact.h"

RandactHandler::RandactHandler() : SerializerHandler() {
    printf("Starting Random Active Testing...\n");
}

RandactHandler::~RandactHandler() {
    printf("Ending Random Active Testing...\n");
}



bool RandactHandler::isLockRace() {
    char type[MAX_READ_SIZE];
    FILE * file_in = fopen("thrille-randomactive", "r");
    safe_assert(file_in != NULL);
    fgets(type, MAX_READ_SIZE, file_in);
    fclose(file_in);
    if (strncmp(type, "LOCK", 4) == 0) {
        return true;
    } else {
        return false;
    }
}

ExecutionTracker * RandactHandler::getNewExecutionTracker(thrID myself) {
    if (isLockRace()) {
        return new RandomLockTester(myself);
    } else {
        return new RandomDataTester(myself);
    }
}
