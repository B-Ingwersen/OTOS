
# Download and build the gcc cross compiler and binutils from source; modify
# BINUTILS_VERSION and GCC_VERSION to change the software versions (see gnu
# mirror site (https://ftp.gnu.org/gnu) to see what versions were available)

# check that sufficient arguments were passed
if [ $# -lt 1 ]
then
    echo "Usage: $0 <path/to/build/cross/compiler>"
    exit 1
fi

# the directory to build the cross compiler in
BUILD_DIR=$1

# set directory paths & cross compiler target
cd $BUILD_DIR
BUILD_DIR_ABSOLUTE=$(pwd "$BUILD_DIR")
PREFIX="$BUILD_DIR_ABSOLUTE/opt/cross"
TARGET="i686-elf"
PATH="$PREFIX/bin:$PATH"

# set binutils version and download location
BINUTILS_VERSION="2.35.1"
BINUTILS_COMPRESSED="binutils-"$BINUTILS_VERSION".tar.xz"

# set gcc version and download location
GCC_VERSION="10.2.0"
GCC_COMPRESSED="gcc-"$GCC_VERSION".tar.xz"

# download and decompress binutils
echo "Downloading bin-utils..."
curl "https://ftp.gnu.org/gnu/binutils/binutils-"$BINUTILS_VERSION".tar.xz" --output $BINUTILS_COMPRESSED
echo "Decompressing bin-utils..."
tar -xf $BINUTILS_COMPRESSED

# build & install binutils
mkdir build-binutils
cd build-binutils
"../binutils-$BINUTILS_VERSION/configure" --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
cd ..

# donwload and decompress gcc
echo "Downloading gcc..."
curl "https://ftp.gnu.org/gnu/gcc/gcc-"$GCC_VERSION"/gcc-"$GCC_VERSION".tar.xz" --output $GCC_COMPRESSED
echo "Decompressing gcc..."
tar -xf $GCC_COMPRESSED

# build & install gcc
mkdir build-gcc
cd build-gcc
"../gcc-$GCC_VERSION/configure" --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
cd ..

# remove source files
echo "Cleaning up..."
rm -f $BINUTILS_COMPRESSED
rm -f $GCC_COMPRESSED
rm -rf "binutils-$BINUTILS_VERSION"
rm -rf "gcc-$GCC_VERSION"
rm -rf build-binutils
rm -rf build-gcc

# move files into BUILD_DIR
mv $PREFIX/* .
rm -rf opt



