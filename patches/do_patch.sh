#!/bin/sh

set -e
THIS_SCRIPT_NAME=`readlink -nf ${0}`
THIS_SCRIPT_DIR=`dirname ${THIS_SCRIPT_NAME}`

for _FILE in `find "${THIS_SCRIPT_DIR}/" -maxdepth 1 -type f -name '*.patch'`; do
	echo "Applying Linux kernel patch: ${_FILE}"
	patch -Nslf -p1 --directory "${THIS_SCRIPT_DIR}/../media_tree" --input "${_FILE}"
done

exit 0
