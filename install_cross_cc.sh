#!/bin/bash
mkdir src
mkdir opt
cd opt
mkdir cross
cd $HOME/src

wget ftp://ftp.gnu.org/gnu/binutils/binutils-2.29.1.tar.xz
wget ftp://ftp.gnu.org/gnu/gcc/gcc-4.9.2/gcc-4.9.2.tar.gz

tar -xvf binutils-2.29.1.tar.xz
tar -xvf gcc-4.9.2.tar.gz

sudo apt-get install libgmp3-dev
sudo apt-get install libmpfr-dev
sudo apt-get install libisl-dev
sudo apt-get install libcloog-isl-dev
sudo apt-get install libmpc-dev
sudo apt-get install texinfo

export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

mkdir build-binutils
cd build-binutils
../binutils-2.29.1/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

cd $HOME/src
 
# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is not in the PATH
 
mkdir build-gcc
cd build-gcc
../gcc-4.9.2/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

cd $HOME

$HOME/opt/cross/bin/$TARGET-gcc --version
export PATH="$HOME/opt/cross/bin:$PATH"

echo done.
