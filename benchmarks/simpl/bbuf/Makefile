include ../common.mk

SRCFILE=src/boundedBuffer.c
OUT=bb


llvm: $(SRCFILE)
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -c -emit-llvm -g $(SRCFILE) -o obj/tmp.bc 
	$(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o obj/tmppass.bc obj/tmp.bc
	$(LLVM_LLC_PATH) -f obj/tmppass.bc
	gcc obj/tmppass.s -o bin/$(OUT) -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread
	rm -f iiddump 


llvmerr1:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -c -DERR1 -emit-llvm -g $(SRCFILE) -o obj/tmp.bc 
	$(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o obj/tmppass.bc obj/tmp.bc
	$(LLVM_LLC_PATH) -f obj/tmppass.bc
	gcc obj/tmppass.s -o bin/$(OUT)-e1 -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread
	rm -f iiddump 

clean:
	rm -f bin/* iiddump obj/*
