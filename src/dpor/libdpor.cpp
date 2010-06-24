#include "libdpor.h"


ExecutionTracker * DporHandler::getNewExecutionTracker(thrID 
        myself){
    return new DporTracker(myself);  
}

