# Makefile for pfscan

include ../common.mk

CFLAGS = -g -D_FILE_OFFSET_BITS=64 -D_REENTRANT -D_GNU_SOURCE

llvm:
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) $(CFLAGS) -DNICKEDIT -c -emit-llvm src/bzip2smp.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/bzip2smp.s
	$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm src/note.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/note.s
	$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm src/detectht.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/detectht.s
	#$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm src/bzlib.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/bzlib.s
	gcc $(CFLAGS) src/bzlib.c -S -o obj/bzlib.s
	#$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm src/compress.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/compress.s
	gcc $(CFLAGS) src/compress.c -S -o obj/compress.s
	#$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm src/decompress.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/decompress.s
	gcc $(CFLAGS) src/decompress.c -S -o obj/decompress.s
	$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm src/crctable.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/crctable.s
	#$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm src/blocksort.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/blocksort.s
	gcc $(CFLAGS) src/blocksort.c -S -o obj/blocksort.s
	$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm src/huffman.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/huffman.s
	$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm src/randtable.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/randtable.s
	gcc  $(CFLAGS) obj/bzip2smp.s obj/note.s obj/detectht.s obj/bzlib.s obj/compress.s obj/decompress.s obj/crctable.s obj/blocksort.s obj/huffman.s obj/randtable.s -o bin/bzip2smp -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread 
	rm iiddump

llvmerr1: 
	mkdir -p obj
	mkdir -p bin
	$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -DNICKEDIT -c -emit-llvm src/bzip2smp.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/bzip2smp.s
	$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm src/note.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/note.s
	$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm src/detectht.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/detectht.s
	#$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm src/bzlib.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/bzlib.s
	gcc $(CFLAGS) -DERR1 src/bzlib.c -S -o obj/bzlib.s
	gcc $(CFLAGS) -DERR1 src/compress.c -S -o obj/compress.s
	#$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm src/compress.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/compress.s
	#$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm src/decompress.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/decompress.s
	gcc $(CFLAGS) src/decompress.c -S -o obj/decompress.s
	$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm src/crctable.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/crctable.s
	#$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm src/blocksort.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/blocksort.s
	gcc $(CFLAGS) src/blocksort.c -S -o obj/blocksort.s
	$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm src/huffman.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/huffman.s
	$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm src/randtable.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o obj/randtable.s
	gcc  $(CFLAGS) -DERR1 obj/bzip2smp.s obj/note.s obj/detectht.s obj/bzlib.s obj/compress.s obj/decompress.s obj/crctable.s obj/blocksort.s obj/huffman.s obj/randtable.s -o bin/bzip2smp-e1 -L$(THRILLE_ROOT)/bin/ -ldummy -lpthread 
	rm iiddump

clean:
	rm -f bin/* iiddump obj/*

