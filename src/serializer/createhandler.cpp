#include "libserializer.h"

Handler * create_handler() {
    return new SerializerHandler();
}
