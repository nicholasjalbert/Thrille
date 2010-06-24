#include "liblockrace.h"

Handler * create_handler() {
    return new LockraceHandler();
}
