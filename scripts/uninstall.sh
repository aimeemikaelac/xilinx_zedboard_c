#!/bin/bash

for entry in $(ls /usr/local/include/uio); do
	rm /usr/local/include/$entry
done

rm -rf /usr/local/include/uio

rm -f /usr/local/lib/libuio.a
