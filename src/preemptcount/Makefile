THRILLERNAME=preemptcount
SRCS=libpreemptcount.cpp fairschedule.cpp preemptcounter.cpp createhandler.cpp ../randomschedule/randomtracker.cpp ../serializer/libserializer.cpp ../serializer/executiontracker.cpp ../serializer/logger.cpp ../serializer/threadtracker.cpp ../serializer/racer.cpp ../serializer/barriertracker.cpp
HEADERS=libpreemptcount.h preemptcounter.h fairschedule.h tlspreemptcount.h ../randomschedule/randomtracker.h ../serializer/libserializer.h  ../serializer/tlsserializer.h  ../serializer/serializertypes.h  ../serializer/logger.h  ../serializer/executiontracker.h  ../serializer/threadtracker.h ../serializer/racer.h ../serializer/barriertracker.h
TARGET=libpreemptcount.so
TESTFILES=libpreemptcount.cpp

include ../thrille-core/core.mk

customclean:
	rm -f $(OBJROOT)/serializer/*.o
	rm -f $(OBJROOT)/seriailzer/*.test
	rm -f $(OBJROOT)/randomschedule/*.o
	rm -f $(OBJROOT)/randomschedule/*.test
