#include "libaddrserial.h"

Handler * create_handler() {
    return new AddrserialHandler();
}
