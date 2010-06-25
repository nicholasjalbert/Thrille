# Makefile for parallel simulated annealer

include ../common.mk

SRC=src/ctrace.foobar.comb.c
OBJ=obj/ctrace.s
TARGET=bin/ctrace

llvm:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -c -emit-llvm -g $(SRC) -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o $(OBJ)
	gcc $(OBJ)  -o $(TARGET) -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread -g 

#Occasional Deadlock
llvmerr1:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -DERR1 -c -emit-llvm -g $(SRC) -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o $(OBJ)
	gcc -DERR1 $(OBJ)  -o $(TARGET)-e1 -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread -g 
	rm iiddump

clean:
	rm -f bin/* iiddump obj/*
