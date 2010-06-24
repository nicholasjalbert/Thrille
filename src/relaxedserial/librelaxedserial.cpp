#include "librelaxedserial.h"

RelaxedserialHandler::RelaxedserialHandler() : SerializerHandler() {
    printf("Thrille:Starting *relaxed* serializer handler...\n");
}

RelaxedserialHandler::~RelaxedserialHandler() {
    printf("Thrille:Ending *relaxed* serializer handler...\n");
}


ExecutionTracker * RelaxedserialHandler::getNewExecutionTracker(thrID 
        myself){
    return new RelaxedTracker(myself);  
}
