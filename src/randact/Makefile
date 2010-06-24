include ../serializer/thriller.mk
include ../randomschedule/thriller.mk
include thriller.mk

SRCS=$(SERIALIZER_SRC_NAMES:%.cpp=../serializer/%.cpp) 
SRCS+=$(RANDOMSCHEDULE_SRC_NAMES:%.cpp=../randomschedule/%.cpp) 
SRCS+=$(RANDACT_SRC_NAMES)
SRCS+=createhandler.cpp
HEADERS=$(SERIALIZER_HEADER_NAMES:%.h=../serializer/%.h) 
HEADERS+=$(RANDOMSCHEDULE_HEADER_NAMES:%.h=../randomschedule/%.h) 
HEADERS+=$(RANDACT_HEADER_NAMES)

include ../thrille-core/core.mk

customclean:
	rm -f $(OBJROOT)/randomschedule/*.o
	rm -f $(OBJROOT)/randomschedule/*.test
	rm -f $(OBJROOT)/serializer/*.o
	rm -f $(OBJROOT)/serializer/*.test    
