#include "libstrictserial.h"

StrictserialHandler::StrictserialHandler() : SerializerHandler() {
    printf("Starting *strict* serializer handler...\n");
}

StrictserialHandler::~StrictserialHandler() {
    printf("Ending *strict* serializer handler...\n");
}


ExecutionTracker * StrictserialHandler::getNewExecutionTracker(thrID 
        myself){
    return new StrictTracker(myself);  
}
