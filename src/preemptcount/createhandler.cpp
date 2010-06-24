#include "libpreemptcount.h"

Handler * create_handler() {
    return new PreemptcountHandler();
}
