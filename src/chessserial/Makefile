include ../serializer/thriller.mk
include thriller.mk

SRCS=$(SERIALIZER_SRC_NAMES:%.cpp=../serializer/%.cpp) 
SRCS+=$(CHESSSERIAL_SRC_NAMES)
SRCS+=createhandler.cpp ../addrserial/fairscheduler.cpp
HEADERS=$(SERIALIZER_HEADER_NAMES:%.h=../serializer/%.h) 
HEADERS+=$(CHESSSERIAL_HEADER_NAMES)
HEADERS+=../addrserial/fairscheduler.h

include ../thrille-core/core.mk

customclean:
	rm -f $(OBJROOT)/serializer/*.o
	rm -f $(OBJROOT)/serializer/*.test
