# Makefile for parallel simulated annealer

include ../common.mk

TARGET=canneal
LIBS:=$(LIBS) -lm

CXXFLAGS+=-DENABLE_THREADS -pthread

llvm:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GPP_PATH) $(CXXFLAGS) -c -emit-llvm -g src/annealer_thread.cpp -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/annealer_thread.s
	$(LLVM_GPP_PATH) $(CXXFLAGS) -c -emit-llvm -g src/rng.cpp -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/rng.s
	$(LLVM_GPP_PATH) $(CXXFLAGS) -c -emit-llvm -g src/netlist.cpp -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/netlist.s
	$(LLVM_GPP_PATH) $(CXXFLAGS) -c -emit-llvm -g src/main.cpp -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/main.s
	$(LLVM_GPP_PATH) $(CXXFLAGS) -c -emit-llvm -g src/netlist_elem.cpp -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/netlist_elem.s
	g++  $(CXXFLAGS) obj/rng.s obj/netlist.s obj/main.s obj/annealer_thread.s obj/netlist_elem.s  -o bin/canneal -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread 
	rm -f *.s iiddump

llvmerr1:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GPP_PATH) $(CXXFLAGS) -DERR1 -c -emit-llvm -g src/annealer_thread.cpp -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/annealer_thread.s
	$(LLVM_GPP_PATH) $(CXXFLAGS) -DERR1 -c -emit-llvm -g src/rng.cpp -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/rng.s
	$(LLVM_GPP_PATH) $(CXXFLAGS) -DERR1 -c -emit-llvm -g src/netlist.cpp -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/netlist.s
	$(LLVM_GPP_PATH) $(CXXFLAGS) -DERR1 -c -emit-llvm -g src/main.cpp -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/main.s
	$(LLVM_GPP_PATH) $(CXXFLAGS) -DERR1 -c -emit-llvm -g src/netlist_elem.cpp -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/netlist_elem.s
	g++  $(CXXFLAGS) -DERR1 obj/rng.s obj/netlist.s obj/main.s obj/annealer_thread.s obj/netlist_elem.s  -o bin/canneal-e1 -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread
	rm -f *.s iiddump

clean:
	rm -f bin/* iiddump obj/*
