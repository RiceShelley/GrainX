set -e
. ./config.sh

mkdir -p "$SYSROOT"

for PROJECT in $SYSTEM_HEADER_PROJECTS; do
	echo ----------------------------
	echo Copying headers for $PROJECT
	echo ----------------------------
	(cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install-headers)
done
