include ../racer/thriller.mk
include thriller.mk

SRCS=$(RACER_SRC_NAMES:%.cpp=../racer/%.cpp) 
SRCS+=$(LOCKRACE_SRC_NAMES)
SRCS+=createhandler.cpp
HEADERS=$(RACER_HEADER_NAMES:%.h=../racer/%.h) 
HEADERS+=$(LOCKRACE_HEADER_NAMES)

include ../thrille-core/core.mk

customclean:
	rm -f $(OBJROOT)/racer/*.o
	rm -f $(OBJROOT)/racer/*.test
