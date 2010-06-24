#include "libchessserial.h"

Handler * create_handler() {
    return new ChessserialHandler();
}
