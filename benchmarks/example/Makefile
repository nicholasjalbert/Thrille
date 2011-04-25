include ../simpl/common.mk

SSRCFILE=simple-test.cpp
SOUT=simple-test

RSRCFILE=simple-race.cpp
ROUT=simple-race

all: llvm-simple llvm-race

llvm-simple: $(SSRCFILE)
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -c -emit-llvm -g $(SSRCFILE) -o obj/tmp.bc 
	$(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o obj/tmppass.bc obj/tmp.bc
	$(LLVM_LLC_PATH) -f obj/tmppass.bc
	gcc obj/tmppass.s -o bin/$(SOUT) -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread
	rm -f iiddump 

llvm-race: $(RSRCFILE)
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -c -emit-llvm -g $(RSRCFILE) -o obj/tmp.bc 
	$(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o obj/tmppass.bc obj/tmp.bc
	$(LLVM_LLC_PATH) -f obj/tmppass.bc
	gcc obj/tmppass.s -o bin/$(ROUT) -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread
	rm -f iiddump 


clean:
	rm -f bin/* iiddump obj/*
