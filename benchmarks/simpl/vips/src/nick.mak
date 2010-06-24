# Makefile for vips
# compiles region.c, semaphore.c, threadgroup.c, and debug.c
# with LLVM so we can track memory accesses

include ../../../../../common.mk

CFLAGS= -std=gnu99 -DHAVE_CONFIG_H -I. -I../.. -I../../libvips/include -pthread -g -I$(THRILLE_ROOT)/benchmarks/simpl/vips/src/vips-libs/lib/glib-2.0/include -I$(THRILLE_ROOT)/benchmarks/simpl/vips/src/vips-libs/include/libxml2 -I$(THRILLE_ROOT)/benchmarks/simpl/vips/src/vips-libs/include/glib-2.0 -MD -MP -MF

llvm:
	$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm region.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f   -o region.s
	gcc -c region.s -o region.o
	rm iiddump
	$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm semaphore.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f   -o semaphore.s
	gcc -c semaphore.s -o semaphore.o
	rm iiddump
	$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm threadgroup.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f   -o threadgroup.s
	gcc -c threadgroup.s -o threadgroup.o
	rm iiddump
	$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm debug.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f   -o debug.s
	gcc -c debug.s -o debug.o
	rm iiddump


