include ../simpl/common.mk

SSRCFILE=simple-test.cpp
SOUT=simple-test

RSRCFILE=simple-race.cpp
ROUT=simple-race

igSRCFILE=simple-signal.cpp
igOUT=simple-signal

all: llvm-simple llvm-race llvm-signal

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

llvm-signal: $(igSRCFILE)
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -c -emit-llvm -g $(igSRCFILE) -o obj/tmp.bc 
	$(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o obj/tmppass.bc obj/tmp.bc
	$(LLVM_LLC_PATH) -f obj/tmppass.bc
	gcc obj/tmppass.s -o bin/$(igOUT) -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread
	rm -f iiddump 


clean:
	rm -f bin/* iiddump obj/* *.save *.schedule my-schedule thrille-sched *.mzn thrille-randomactive
