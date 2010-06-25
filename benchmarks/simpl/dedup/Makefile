# Makefile for dedup kernel

include ../common.mk

TARGET=bin/dedup

BIGFILE_CFLAGS=$(shell getconf LFS_CFLAGS)

CFLAGS += -g -Wall -std=c99 -fno-strict-aliasing -D_XOPEN_SOURCE=500
LIBS += -lz -lm -lssl

DEDUP_OBJ =  hashtable.o queue.o util.o binheap.o tree.o decoder.o dedup.o rabin.o encoder.o
CFLAGS += -DPARALLEL -pthread

all:llvm

llvm:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/hashtable.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/hashtable.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/queue.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/queue.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/util.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/util.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/binheap.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/binheap.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/tree.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/tree.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/decoder.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/decoder.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/dedup.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/dedup.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/rabin.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/rabin.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/encoder.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/encoder.s
	gcc  obj/hashtable.s obj/queue.s obj/util.s obj/binheap.s obj/tree.s obj/decoder.s obj/dedup.s obj/rabin.s obj/encoder.s -o bin/dedup -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread -lz -lm -lssl
	rm iiddump


llvmerr1:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/hashtable.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/hashtable.s
#	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) -DNICKSLEEP -DERR1 src/queue.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/queue.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) -DERR1 src/queue.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/queue.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/util.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/util.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/binheap.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/binheap.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/tree.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/tree.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/decoder.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/decoder.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/dedup.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/dedup.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/rabin.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/rabin.s
#	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) -DNICKSLEEP src/encoder.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/encoder.s
	$(LLVM_GCC_PATH) -c -emit-llvm $(CFLAGS) src/encoder.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/encoder.s
	gcc  obj/hashtable.s obj/queue.s obj/util.s obj/binheap.s obj/tree.s obj/decoder.s obj/dedup.s obj/rabin.s obj/encoder.s -o bin/dedup-e1 -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread -lz -lm -lssl
	rm iiddump

clean:
	rm -f bin/* iiddump obj/*
