#!/bin/sh
set -e
. ./headers.sh

for PROJECT in $PROJECTS; do
	echo -----------------------------
	echo Copying BINARIES for $PROJECT
	echo -----------------------------
	(cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)
done
