PREFIX=${PARSECDIR}/pkgs/apps/blackscholes/inst/${PARSECPLAT}

include ../common.mk

ifdef source
	ifeq "$(source)" "simd" 
		SRC = blackscholes.simd.c
	endif
else
	SRC	= blackscholes.c
endif

CSRC    = $(SRC)
TARGET  = blackscholes 
M4_BASE = .
MACROS  = c.m4.pthreads

M4_SRC    = blackscholes.m4.cpp
CSRC      = $(M4_SRC)
MT        = -DENABLE_THREADS
CXXFLAGS += -pthread

# Default build Double precision version
NCO     = -DNCO=2

ifdef chk_err
ERR     = -DERR_CHK
endif

ifdef func
FUNC = -DFUNC_EXE
endif

ifdef single
NCO = -DNCO=4
endif


ifdef size
SZ = -DN=$(size)
else
SZ = -DN=960
endif

ifdef double
NCO = -DNCO=2
endif

CXXFLAGS += $(MT) $(SZ) $(NCO) $(FUNC) $(ERR)

all: llvm

llvm:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GPP_PATH) -c -emit-llvm -g $(CXXFLAGS) src/$(CSRC) -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/blackscholes.s
	g++ $(CXXFLAGS) obj/blackscholes.s  -o bin/blackscholes -L$(THRILLE_ROOT)/bin/ -ldummy 
	rm -f *.s iiddump

llvmerr1:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GPP_PATH) -c -emit-llvm -g $(CXXFLAGS) src/$(CSRC)  -DERR1 -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/blackscholes.s
	g++ $(CXXFLAGS) obj/blackscholes.s -o bin/blackscholes-e1 -L$(THRILLE_ROOT)/bin/ -ldummy 
	rm -f *.s iiddump


clean:
	rm -f bin/* iiddump obj/*

