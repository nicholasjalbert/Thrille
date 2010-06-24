

# Builds VIPS dependencies and then VIPS itself.
# if command line arg $1 is nonzero and nonempty, it will build the bug
#
# Author: Nick Jalbert (jalbert@eecs.berkeley.edu)

BUILD_BUG=1

if [ "0" = "$1" ]
then 
    echo "Not building VIPS with bug..."
    BUILD_BUG=0
else
    echo "**BUILDING** VIPS with bug..."
    BUILD_BUG=1
fi

echo

WORKING_DIR=`pwd`
VIPS_LIB_PATH="$WORKING_DIR/vips-libs"
VIPS_LINKER_LIB_PATH="$VIPS_LIB_PATH/lib"
VIPS_INSTALL_PATH="$WORKING_DIR/vips-install"

if [ -d "gettext-0.17" ] 
then
    echo "Not expanding gettext--directory exists"
else
    echo "Expanding gettext directory..."
    tar xzf gettext-0.17.tar.gz
fi

if [ -d "glib-2.22.4" ] 
then
    echo "Not expanding glib--directory exists"
else
    echo "Expanding glib directory..."
    tar xzf glib-2.22.4.tar.gz
fi

if [ -d "libxml2-2.7.7" ] 
then
    echo "Not expanding libxml2--directory exists"
else
    echo "Expanding libxml2 directory..."
    tar xzf libxml2-2.7.7.tar.gz
fi

if [ -d "zlib-1.2.4" ]
then
    echo "Not expanding zlib--directory exists"
else
    echo "Expanding zlib directory..."
    tar xzf zlib-1.2.4.tar.gz
fi

if [ -d "vips-7.20.0" ]
then
    echo "Not expanding vips--directory exists"
else
    echo "Expanding vips directory..."
    tar xzf vips-7.20.0.tar.gz
fi

echo "Making library install dir: $VIPS_LIB_PATH"
mkdir -p $VIPS_LIB_PATH

echo "Making vips install dir: $VIPS_INSTALL_PATH"
mkdir -p $VIPS_INSTALL_PATH

export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$VIPS_LINKER_LIB_PATH/pkgconfig"
export CFLAGS="-g -static"
export CXXFLAGS="-g -static"

echo
echo "Building gettext..."
python gettext-fix.py
BLOG=gettext-build.log
cd gettext-0.17
if [ -e "$BLOG" ]
then
    echo "gettext build log exists--assume it has been built"
else
    echo "Configuring gettext"
    ./configure --disable-shared --prefix=$VIPS_LIB_PATH > $BLOG 2>&1
    echo "Building gettext..."
    make >> $BLOG 2>&1
    echo "Installing gettext..."
    make install >> $BLOG 2>&1
    echo "Build log can be found in gettext-0.17/$BLOG"
fi

echo
echo "Building glib..."
BLOG=glib-build.log
cd ../glib-2.22.4
if [ -e "$BLOG" ]
then
    echo "glib build log exists--assume it has been built"
else
    echo "Configuring glib"
    ./configure --disable-shared --prefix=$VIPS_LIB_PATH > $BLOG 2>&1
    echo "Building glib..."
    make >> $BLOG 2>&1
    echo "Installing glib..."
    make install >> $BLOG 2>&1
    echo "Build log can be found in glib-2.22.4/$BLOG"
fi


echo
echo "Building libxml2..."
cd ../libxml2-2.7.7
BLOG=libxml2-build.log
if [ -e "$BLOG" ]
then
    echo "libxml2 build log exists--assume it has been built"
else
    echo "Configuring libxml2"
    ./configure --disable-shared --prefix=$VIPS_LIB_PATH > $BLOG 2>&1
    echo "Building libxml2..."
    make >> $BLOG 2>&1
    echo "Installing libxml2..."
    make install >> $BLOG 2>&1
    echo "Build log can be found in libxml2-2.7.7/$BLOG"
fi

echo
echo "Building zlib..."
cd ../zlib-1.2.4
BLOG=zlib-build.log
if [ -e "Makefile" ]
then
    echo "zlib build log exists--assume it has been built"
else
    echo "Configuring zlib"
    ./configure --disable-shared --prefix=$VIPS_LIB_PATH > $BLOG 2>&1
    echo "Building zlib..."
    make >> $BLOG 2>&1
    echo "Installing zlib..."
    make install >> $BLOG 2>&1
    echo "Build log can be found in zlib-1.2.4/$BLOG"
fi


echo
echo "Building vips..."
cd ..

python vips-fix.py

if [ "0" = "$BUILD_BUG" ]
then 
    python vips-cleanbug.py
else
    python vips-seedbug.py
fi

cd vips-7.20.0

BASIC_CONF="--prefix=$VIPS_INSTALL_PATH --disable-shared --without-fftw3 --without-magick --without-liboil --without-lcms --without-OpenEXR --without-pangoft2 --without-tiff --without-jpeg --without-zip --without-png --without-libexif --without-python --without-x --without-perl --without-v4l --without-cimg"

BUILD_CONF="$BASIC_CONF --enable-threads"

BLOG=vips-build.log

export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$VIPS_LINKER_LIB_PATH/pkgconfig"
export LDFLAGS="-L$VIPS_LINKER_LIB_PATH -L$THRILLE_ROOT/bin -L/usr/local/lib64"
export LIBS="$THRILLE_ROOT/bin/libdummy.so -lstdc++" 
export CFLAGS="-g"
export CXXFLAGS="-g"
export VIPS_CFLAGS="-g" 

rm -f Makefile
echo "Configure\n" > $BLOG 2>&1
echo >> $BLOG 2>&1
./configure $BUILD_CONF >> $BLOG 2>&1

echo >> $BLOG 2>&1
echo "Make Clean\n" >> $BLOG 2>&1
echo >> $BLOG 2>&1
make clean >> $BLOG 2>&1
echo >> $BLOG 2>&1
echo "Make\n" >> $BLOG 2>&1
echo >> $BLOG 2>&1
make  >> $BLOG 2>&1
echo >> $BLOG 2>&1
echo "Make Install\n" >> $BLOG 2>&1
echo >> $BLOG 2>&1
make install  >> $BLOG 2>&1

cp ../nick.mak libvips/iofuncs/

cd libvips/iofuncs
echo >> $BLOG 2>&1
echo "Compile with LLVM\n" >> $BLOG 2>&1
echo >> $BLOG 2>&1
make -f nick.mak >> $BLOG 2>&1
touch im_cp_desc.c
cd ../..


echo >> $BLOG 2>&1
echo "Remake\n" >> $BLOG 2>&1
echo >> $BLOG 2>&1
make >> $BLOG 2>&1
echo >> $BLOG 2>&1
echo "Remake and install\n" >> $BLOG 2>&1
echo >> $BLOG 2>&1
make install >> $BLOG 2>&1
echo >> $BLOG 2>&1
echo "Build log can be found in vips-7.20.0/$BLOG"

if [ "0" = "$BUILD_BUG" ]
then 
    echo
    echo "Creating file bin/vips"
    cp ../vips-install/bin/vips ../../bin/vips
else
    echo
    echo "Creating file bin/vips-e1"
    cp ../vips-install/bin/vips ../../bin/vips-e1
fi
