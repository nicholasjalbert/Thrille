#include "libpreemptcount.h"

PreemptcountHandler::PreemptcountHandler() : SerializerHandler() {
    dbgPrint("Starting Preemption Counting Thriller\n");
}

PreemptcountHandler::~PreemptcountHandler() {
    dbgPrint("Ending Preemption Counting Thriller\n");
}

ExecutionTracker * PreemptcountHandler::getNewExecutionTracker(thrID main) {
    return new PreemptionCounter(main);
}

