#include "libblank.h"

Handler * create_handler() {
    return new BlankHandler();
}
