#!/bin/bash
for i in $(seq 10 100 13000)
do
	echo $i
	./aes.o "$i"
done
