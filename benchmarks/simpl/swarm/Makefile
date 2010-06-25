include ../common.mk

SRCFILE=swarm_isort64.comb.c
OUT=swarm


llvm: src/$(SRCFILE)
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -c -emit-llvm -g src/$(SRCFILE) -o bin/tmp.bc 
	$(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o bin/tmppass.bc bin/tmp.bc
	$(LLVM_LLC_PATH) -f bin/tmppass.bc
	g++ bin/tmppass.s -o bin/$(OUT) -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread -lbz2 
	rm -f iiddump bin/*.s bin/*.bc	

llvmerr1: src/$(SRCFILE)
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -c -emit-llvm -g -DERR1 src/$(SRCFILE) -o bin/tmp.bc 
	$(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o bin/tmppass.bc bin/tmp.bc
	$(LLVM_LLC_PATH) -f bin/tmppass.bc
	g++ bin/tmppass.s -o bin/$(OUT)-e1 -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread -lbz2 
	rm -f iiddump bin/*.s bin/*.bc	

clean:
	rm -f bin/* iiddump obj/*
