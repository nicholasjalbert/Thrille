#include "libaddrserial.h"

AddrserialHandler::AddrserialHandler() : SerializerHandler() {
    printf("Starting *addr* serializer handler...\n");
}

AddrserialHandler::~AddrserialHandler() {
    printf("Ending *addr* serializer handler...\n");
}


ExecutionTracker * AddrserialHandler::getNewExecutionTracker(thrID 
        myself){
    return new AddrTracker(myself);  
}
