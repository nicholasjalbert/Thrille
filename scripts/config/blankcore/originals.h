#ifndef _ORIGINALS_H_INCLUDED
#define _ORIGINALS_H_INCLUDED
#include "coretypes.h"

class Originals
{
    public:
        static void initialize();
        static volatile bool is_initialized();

        PYTHON_STATIC_PUBLIC


    private:
        static void _initialize();
        static volatile bool _initialized;

        PYTHON_STATIC_PRIVATE

};

#endif 
