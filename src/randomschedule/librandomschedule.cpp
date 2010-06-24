#include "librandomschedule.h"
RandomscheduleHandler::RandomscheduleHandler() : SerializerHandler() {
    printf("Starting Randomschedule Thriller...\n");
}

RandomscheduleHandler::~RandomscheduleHandler() {
    printf("Ending Randomschedule Thriller...\n");
}


ExecutionTracker * RandomscheduleHandler::getNewExecutionTracker(thrID 
        myself){
    return new RandomTracker(myself);  
}
