include ../serializer/thriller.mk
include thriller.mk

SRCS=$(SERIALIZER_SRC_NAMES:%.cpp=../serializer/%.cpp) 
SRCS+=$(RELAXEDSERIAL_SRC_NAMES)
SRCS+=createhandler.cpp
HEADERS=$(SERIALIZER_HEADER_NAMES:%.h=../serializer/%.h) 
HEADERS+=$(RELAXEDSERIAL_HEADER_NAMES)

include ../thrille-core/core.mk

customclean:
	rm -f $(OBJROOT)/serializer/*.o
	rm -f $(OBJROOT)/serializer/*.test
