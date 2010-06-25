include ../common.mk

SRCFILE=pbzip2.cpp
OUT=pbzip2


# pbzip2 with instrumented memory operations
llvm: src/pbzip2.cpp
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GPP_PATH) -c -emit-llvm -g src/$(SRCFILE) -o bin/tmp.bc 
	$(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o bin/tmppass.bc bin/tmp.bc
	$(LLVM_LLC_PATH) -f bin/tmppass.bc
	g++ bin/tmppass.s -o bin/$(OUT) -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread -lbz2 
	rm iiddump bin/*.s bin/*.bc	

llvmerr1: 
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GPP_PATH) -c -emit-llvm -g src/$(SRCFILE) -o bin/tmp.bc 
	$(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o bin/tmppass.bc bin/tmp.bc
	$(LLVM_LLC_PATH) -f bin/tmppass.bc
	g++ bin/tmppass.s -o bin/$(OUT)-e1 -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread -lbz2 
	rm iiddump bin/*.s bin/*.bc	

clean:
	rm -f bin/* iiddump obj/*
