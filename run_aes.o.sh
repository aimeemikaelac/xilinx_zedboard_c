#!/bin/bash

if [ -z "$1" ]; then
	iter=1
else
	iter=$1
fi

output/gpio.o -g 0x43c10014 -o 1

output/gpio.o -g 0x43c10014 -o 0

output/aes.o $iter
