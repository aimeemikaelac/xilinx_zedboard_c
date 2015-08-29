#!/bin/bash

set +e

if [ -d "$/usr/local/include/uio" ]; then
	for entry in $(ls /usr/local/include/uio); do
		rm -v /usr/local/include/$entry
	done
fi

rm -rfv /usr/local/include/uio

rm -fv /usr/local/lib/libuio.a
rm -fv /usr/local/lib/libuio.so
