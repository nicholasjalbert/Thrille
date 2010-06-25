THRILLEDIR=$(THRILLE_ROOT)/src/thrille-core
BINDIR=$(THRILLE_ROOT)/bin
TESTDIR=$(THRILLE_ROOT)/tests/$(THRILLERNAME)
THRILLEHEADERS=$(addprefix $(THRILLEDIR)/, libpth.h originals.h coretypes.h threadtracker.h) 
THRILLESRCS=libpth.cpp originals.cpp threadtracker.cpp
THRILLEOBJDIR=$(THRILLE_ROOT)/obj/thrille-core
THRILLEOBJS=$(THRILLESRCS:%.cpp=$(THRILLEOBJDIR)/%.o)
OBJDIR=$(THRILLE_ROOT)/obj/$(THRILLERNAME)
OBJROOT=$(THRILLE_ROOT)/obj

OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o) $(THRILLEOBJS)
TESTNAMES = $(addprefix test, $(TESTFILES:%.cpp=%))
TESTEXEC=$(addsuffix .test, $(TESTNAMES))
TESTSOURCE=$(addsuffix .cpp, $(TESTNAMES))
TESTHEADER=$(addsuffix .h, $(TESTNAMES))

all: objdir $(BINDIR)/$(TARGET) $(BINDIR)/libdummy.so 

$(BINDIR)/$(TARGET): $(OBJS)
	g++ -shared -g -Wall -fPIC -fno-exceptions -o $@ $^ -ldl

$(THRILLEOBJDIR)/%.o: $(THRILLEDIR)/%.cpp $(THRILLEHEADERS)
	g++ -c -g -Wall -fPIC -fno-exceptions -o $@ $<

$(OBJDIR)/%.o: %.cpp $(HEADERS) 
	g++ -c -g -Wall -fPIC -fno-exceptions -o $@ $<

objdir:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJROOT)/thrille-core

$(BINDIR)/libdummy.so: $(THRILLEDIR)/dummy.cpp 
	g++ -shared -g -Wall -fPIC -o $(BINDIR)/libdummy.so \
	    $(THRILLEDIR)/dummy.cpp
	g++ -g -Wall -c -o $(OBJROOT)/thrille-core/libdummy.o \
	    $(THRILLEDIR)/dummy.cpp
	ar rcs $(BINDIR)/libdummy.a $(OBJROOT)/thrille-core/libdummy.o

clean: customclean
	rm -f $(OBJDIR)/*.o
	rm -f $(OBJDIR)/*.test
	rm -f $(THRILLEOBJDIR)/*.o
	rm -f $(BINDIR)/libdummy.so
	rm -f $(BINDIR)/libdummy.a
	rm -f $(BINDIR)/$(TARGET)

test: $(addprefix $(OBJDIR)/, $(TESTEXEC))
	@if [ -d config ]; then ./config/testsetup.sh; fi
	@echo ***BEGIN TESTS***
	@for i in $(TESTEXEC); do \
	    echo Running test $$i; \
	    $(OBJDIR)/$$i; \
	done
	@if [ -d config ]; then ./config/testteardown.sh; fi

$(OBJDIR)/%.test: $(TESTDIR)/%.cpp $(OBJS)
	g++ -I $(THRILLE_ROOT)/tests/cxxtest/ -o $@ $^ -ldl -lpthread

$(TESTDIR)/%.cpp: $(TESTDIR)/%.h
	python $(THRILLE_ROOT)/tests/cxxtest/cxxtestgen.py -o $@ \
	    --error-printer --have-eh $<




