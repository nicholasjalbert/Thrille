#include "libdfailserial.h"

DfailserialHandler::DfailserialHandler() : AddrserialHandler() {
    printf("Starting addr serializer handler (with fail on disable)...\n");
}

DfailserialHandler::~DfailserialHandler() {
    printf("Ending addr serializer handler (with fail on disable)...\n");
}


ExecutionTracker * DfailserialHandler::getNewExecutionTracker(
        thrID myself){
    return new DfailTracker(myself);  
}
