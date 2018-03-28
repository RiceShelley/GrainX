#!/bin/bash
./clean.sh
./iso.sh
qemu-system-i386 -cdrom GrainX.iso -hda hda.img
echo done.
