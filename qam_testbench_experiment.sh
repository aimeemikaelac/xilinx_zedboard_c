#!/bin/bash
for i in $(seq 10 100 13000)
do
	echo $i
	./qam_testbench_static_arm.o "$i"
done
