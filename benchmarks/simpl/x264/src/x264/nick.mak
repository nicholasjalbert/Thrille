# Makefile for x264

include ../../../common.mk

CFLAGS= -Wall -I. -DHAVE_MALLOC_H -DARCH_X86_64 -DSYS_LINUX -DHAVE_PTHREAD -g 


llvm:
	$(LLVM_GCC_PATH) $(CFLAGS) -DERR1 -c -emit-llvm encoder/lookahead.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f   -o encoder/lookahead.s
	gcc -c encoder/lookahead.s -o encoder/lookahead.o
	$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm encoder/encoder.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f  -o encoder/encoder.s
	gcc -c encoder/encoder.s -o encoder/encoder.o
	rm iiddump

comment:
	#$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm encoder/encoder.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f  -o encoder/encoder.s
	#gcc -c encoder/encoder.s -o encoder/encoder.o
	#$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm encoder/ratecontrol.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f -o encoder/ratecontrol.s
	#gcc -c encoder/ratecontrol.s -o encoder/ratecontrol.o
	#$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm encoder/analyse.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f  -o encoder/analyse.s
	#$(LLVM_GCC_PATH) $(CFLAGS) -c -emit-llvm common/frame.c -o - | $(LLVM_OPT_PATH) -load $(LLVM_PASS_PATH) -loadstore -f -o - | $(LLVM_LLC_PATH) -f  -o common/frame.s
	#gcc -c common/frame.s -o common/frame.o

