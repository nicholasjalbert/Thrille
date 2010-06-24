THRILLERNAME=irace
SRCS=libirace.cpp createhandler.cpp ../serializer/threadtracker.cpp interpose.cpp iracer.cpp ../serializer/racer.cpp
HEADERS=libirace.h tlsirace.h ../serializer/threadtracker.cpp ../serializer/serializertypes.h iracer.h iracetypes.h ../serializer/racer.h
TARGET=libirace.so
TESTFILES=libirace.cpp

include ../thrille-core/core.mk

customclean:
	rm -f $(OBJROOT)/serializer/*.o
	rm -f $(OBJROOT)/serializer/*.test    
