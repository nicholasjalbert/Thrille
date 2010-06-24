#!/bin/sh

# Cleans the vips build
#
# Author: Nick Jalbert (jalbert@eecs.berkeley.edu)
#

WORKING_DIR=`pwd`
VIPS_LIB_PATH="$WORKING_DIR/vips-libs"
VIPS_LINKER_LIB_PATH="$VIPS_LIB_PATH/lib"
VIPS_INSTALL_PATH="$WORKING_DIR/vips-install"

echo "Removing vips install dir..."
rm -rf vips-install/ 

echo "Removing vips libraries dir..."
rm -rf vips-libs/ 

echo "Removing vips dir..."
rm -rf vips-7.20.0 

echo "Removing gettext dir..."
rm -rf gettext-0.17

echo "Removing zlib dir..."
rm -rf zlib-1.2.4 

echo "Removing libxml2 dir..."
rm -rf libxml2-2.7.7 

echo "Removing glib dir..."
rm -rf glib-2.22.4

echo "Removing glib build log..."
rm -rf glib-build.log

echo "Removing libxml2 build log..."
rm -rf libxml2-build.log

echo "Removing zlib build log..."
rm -rf zlib-build.log

echo "Removing vips build log..."
rm -rf vips-build.log

