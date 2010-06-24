#include "originals.h"

#define init_original(f, type) { \
    _##f = (type) dlsym(RTLD_NEXT, #f); \
    if (_##f == NULL) { \
        printf("Thrille: originals %s init fail\n", #f); \
    }\
}

volatile bool Originals::_initialized = false;

PYTHON_INIT_NULL

volatile bool Originals::is_initialized()
{
    return _initialized;
}

void Originals::_initialize()
{
    safe_assert(!_initialized);

    PYTHON_INIT_ORIGINALS
    
    _initialized = true;
}


void Originals::initialize()
{
    safe_assert(!_initialized);

    _initialize();

    safe_assert(_initialized);
}

PYTHON_ORIG_IMPL


