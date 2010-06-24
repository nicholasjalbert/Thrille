#include "librelaxedserial.h"

Handler * create_handler() {
    return new RelaxedserialHandler();
}
