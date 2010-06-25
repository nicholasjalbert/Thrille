# Makefile for VIPS, relies on custom bash script in vips/src/nick-make.sh

include ../common.mk

llvm:
	mkdir -p obj
	mkdir -p bin
	@cd src/x264; make; cp x264 ../../bin/

llvmerr1:
	mkdir -p obj
	mkdir -p bin
	@cd src/x264; ./nick_make.sh

clean:
	rm -f bin/* iiddump obj/*

cleanall: clean
	@cd src; \
	    ./nick-clean.sh
