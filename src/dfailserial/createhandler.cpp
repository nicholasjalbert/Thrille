#include "libdfailserial.h"

Handler * create_handler() {
    return new DfailserialHandler();
}
