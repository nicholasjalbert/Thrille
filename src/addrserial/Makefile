include ../serializer/thriller.mk
include ../relaxedserial/thriller.mk
include thriller.mk

SRCS=$(SERIALIZER_SRC_NAMES:%.cpp=../serializer/%.cpp) 
SRCS+=$(RELAXEDSERIAL_SRC_NAMES:%.cpp=../relaxedserial/%.cpp) 
SRCS+=$(ADDRSERIAL_SRC_NAMES)
SRCS+=createhandler.cpp
HEADERS=$(SERIALIZER_HEADER_NAMES:%.h=../serializer/%.h) 
HEADERS+=$(RELAXEDSERIAL_HEADER_NAMES:%.h=../relaxedserial/%.h) 
HEADERS+=$(ADDRSERIAL_HEADER_NAMES)

include ../thrille-core/core.mk

customclean:
	rm -f $(OBJROOT)/relaxedserial/*.o
	rm -f $(OBJROOT)/relaxedserial/*.test
	rm -f $(OBJROOT)/serializer/*.o
	rm -f $(OBJROOT)/serializer/*.test
