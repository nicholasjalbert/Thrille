#include "librelaxedtester.h"

RelaxedtesterHandler::RelaxedtesterHandler() : SerializerHandler() {
    printf("Starting relaxed *TESTER* handler...\n");
}

RelaxedtesterHandler::~RelaxedtesterHandler() {
    printf("Ending relaxed *TESTER* handler...\n");
}

ExecutionTracker * RelaxedtesterHandler::getNewExecutionTracker(thrID 
        myself){
    return new TesterTracker(myself);  
}
