#include "librandact.h"

RandactHandler::RandactHandler() : SerializerHandler() {
    dbgPrint("Starting Random Active Testing...\n");
}

RandactHandler::~RandactHandler() {
    dbgPrint("Ending Random Active Testing...\n");
}



bool RandactHandler::isLockRace() {
    ifstream infile;
    infile.open("thrille-randomactive");
    if (!infile.is_open()) {
        printf("Error, unable to open random active testing target file\n");
        exit(2);
    }
    string type;
    infile >> type;
    infile.close();
    if (type == "LOCK") {
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
