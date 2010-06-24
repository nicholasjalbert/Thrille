#include ../parentthriller/thriller.mk
include thriller.mk


PY**SRCS
#SRCS+=$(PARENTTHRILLER_SRC_NAMES:%.cpp=../parentthriller/%.cpp)

SRCS+=createhandler.cpp

PY**HEADERS
#HEADERS+=$(PARENTTHRILLER_HEADER_NAMES:%.h=../parentthriller/%.h)


include ../thrille-core/core.mk

customclean:
	@#rm -f $(OBJROOT)/serializer/*.o
	@#rm -f $(OBJROOT)/seriailzer/*.test
