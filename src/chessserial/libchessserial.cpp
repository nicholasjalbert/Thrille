#include "libchessserial.h"

ChessserialHandler::ChessserialHandler() : SerializerHandler() {
    printf("Starting *chess* serializer handler...\n");
}

ChessserialHandler::~ChessserialHandler() {
    printf("Ending *chess* serializer handler...\n");
}


ExecutionTracker * ChessserialHandler::getNewExecutionTracker(thrID 
        myself){
    return new ChessTracker(myself);  
}
