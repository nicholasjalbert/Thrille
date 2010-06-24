#include "libstrictserial.h"

Handler * create_handler() {
    return new StrictserialHandler();
}
