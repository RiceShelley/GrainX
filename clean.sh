#!/bin/sh
set -e
. ./headers.sh

for PROJECT in $PROJECTS; do
    echo -------------------------------
    echo cleaning BINARIES for $PROJECT
    echo -------------------------------
    (cd $PROJECT && $MAKE clean)
done