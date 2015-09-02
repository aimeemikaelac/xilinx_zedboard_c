#!/bin/bash

set +e

mkdir -pv /usr/local/include/uio

cp -v src/*.h /usr/local/include/uio
cp -v src/**/*.h /usr/local/include/uio
cp -v /usr/local/include/uio/* /usr/local/include

cp -v output/libuio.so /usr/local/lib
cp -v output/libuio.a /usr/local/lib

ldconfig
