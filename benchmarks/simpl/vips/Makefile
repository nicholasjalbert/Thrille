# Makefile for VIPS, relies on custom bash script in vips/src/nick-make.sh

include ../common.mk

llvm:
	@cd src; \
	    ./nick-make.sh 0

llvmerr1:
	@cd src; \
	    ./nick-make.sh 1

clean:
	rm -f bin/* iiddump obj/*

cleanall: clean
	@cd src; \
	    ./nick-clean.sh
