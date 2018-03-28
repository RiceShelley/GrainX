#!/bin/sh
set -e 

echo BUILDING GRAINX!!!

. ./build.sh

echo ---------------------
echo BUILD FINISHED
echo ---------------------
echo CREATING BOOTABLE ISO
echo ---------------------

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/GrainX.kernel isodir/boot/GrainX.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "GrainX" {
	multiboot /boot/GrainX.kernel
}
EOF
grub-mkrescue -o GrainX.iso isodir
