#!/bin/bash

mkdir -p /usr/local/include/uio

cp src/**/*.h /usr/local/include/uio
cp src/**/*.h /usr/local/include

cp output/libuio.a /usr/local/lib
