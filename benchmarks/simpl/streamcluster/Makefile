# Makefile for streamcluster

include ../common.mk

TARGET=bin/streamcluster

CXXFLAGS :=$(CXXFLAGS) -DENABLE_THREADS -pthread


llvm:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GPP_PATH) -c -emit-llvm -g  -DENABLE_THREADS src/streamcluster.cpp -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/streamcluster.s
	g++ obj/streamcluster.s -o $(TARGET) -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread
	rm -f iiddump *.s

llvmerr1:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GPP_PATH) -c -emit-llvm -g -DERR1 -DENABLE_THREADS src/streamcluster.cpp -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/streamcluster.s
	g++ obj/streamcluster.s -o $(TARGET)-e1 -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread
	rm -f iiddump *.s

clean:
	rm -f bin/* iiddump obj/*

