THRILLERNAME=randact
SRCS=librandact.cpp activeraceinfo.cpp createhandler.cpp interpose.cpp randomactive.cpp randomdata.cpp randomlock.cpp ../randomschedule/randomtracker.cpp ../serializer/libserializer.cpp ../serializer/executiontracker.cpp ../serializer/logger.cpp ../serializer/threadtracker.cpp ../serializer/racer.cpp ../serializer/barriertracker.cpp
HEADERS=librandact.h tlsrandact.h activeraceinfo.h randomactive.h randomdata.h randomlock.h ../randomschedule/randomtracker.h ../serializer/libserializer.h  ../serializer/tlsserializer.h  ../serializer/serializertypes.h  ../serializer/logger.h  ../serializer/executiontracker.h  ../serializer/threadtracker.h ../serializer/racer.h ../serializer/barriertracker.h 
TARGET=librandact.so
TESTFILES=librandact.cpp randomactive.cpp randomdata.cpp randomlock.cpp


include ../thrille-core/core.mk

customclean:
	rm -f $(OBJROOT)/randomschedule/*.o
	rm -f $(OBJROOT)/randomschedule/*.test
	rm -f $(OBJROOT)/serializer/*.o
	rm -f $(OBJROOT)/serializer/*.test    
