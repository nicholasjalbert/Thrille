# Makefile for pfscan

include ../common.mk
DESTDIR=/usr/local

BINDIR=$(DESTDIR)/bin
MANDIR=$(DESTDIR)/man
MAN1DIR=$(MANDIR)/man1

TAR=tar
GZIP=gzip
MAKE=make
INSTALL=./install-sh


## Solaris 8 with Gcc 3.0
GSO_CC=gcc -Wall -g -O -pthreads
GSO_LDOPTS=
GSO_LIBS= -lnsl -lsocket

## Solaris 8 with Forte C 6.2
SOL_CC=cc -mt -O
SOL_LDOPTS=
SOL_LIBS= -lpthread -lnsl -lsocket

## Linux 2.4 with Gcc 2.96
LNX_CC=gcc -Wall -g -O
LNX_LDOPTS=-Wl,-s 
LNX_LIBS=-lpthread -lnsl


OBJS = pfscan.o bm.o version.o pqueue.o


llvm:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -c -emit-llvm -g src/pfscan.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/pfscan.s
	$(LLVM_GCC_PATH) -c -emit-llvm -g src/bm.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/bm.s
	$(LLVM_GCC_PATH) -c -emit-llvm -g src/version.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/version.s
	$(LLVM_GCC_PATH) -c -emit-llvm -g src/pqueue.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/pqueue.s	
	gcc  obj/pfscan.s obj/bm.s obj/version.s obj/pqueue.s  -o bin/pfscan -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread -lnsl
	rm -f *.s iiddump

llvmerr1: 
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -c -emit-llvm -g -DERR1 src/pfscan.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/pfscan.s
	$(LLVM_GCC_PATH) -c -emit-llvm -g -DERR1 src/bm.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/bm.s
	$(LLVM_GCC_PATH) -c -emit-llvm -g -DERR1 src/version.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/version.s
	$(LLVM_GCC_PATH) -c -emit-llvm -g -DERR1 src/pqueue.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/pqueue.s	
	gcc  obj/pfscan.s obj/bm.s obj/version.s obj/pqueue.s  -o bin/pfscan-e1 -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread -lnsl
	rm -f *.s iiddump

clean:
	rm -f bin/* iiddump obj/*

