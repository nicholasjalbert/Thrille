#include "libdpor.h"

Handler * create_handler() {
    return new DporHandler();
}
