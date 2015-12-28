#!/usr/bin/python
import argparse
import os
import timeit

if __name__ == "__main__":
	argparser = argparse.ArgumentParser()
	argparser.add_argument("-f", help="Partial bitstream", type=str, required=True)
	argparser.add_argument("-n", help="Number of iterations", type=int, default=1, required=False)

	args = argparser.parse_args()

	if not os.path.isfile(args.f):
		print "File does not exist: "+str(args.f)
		exit(-1)

	running_total = 0

	for i in range(args.n):
		start_time = timeit.default_timer()
	
		is_partial_file = open("/sys/class/xdevcfg/xdevcfg/device/is_partial_bitstream", "w")
		is_partial_file.write("1")
		is_partial_file.close()
	
		bitstream_file = open(args.f, "r")
		bitstream = bitstream_file.read()
		bitstream_file.close()
	
		xdev_file = open("/dev/xdevcfg", "w")
		xdev_file.write(bitstream)
		xdev_file.close()
	
		prog_file = open("/sys/class/xdevcfg/xdevcfg/device/prog_done" ,"r")
		prog_done = prog_file.read()
		prog_file.close()
	
		while str(prog_done).strip() != "1":
			prog_file = open("/sys/class/xdevcfg/xdevcfg/device/prog_done" ,"r")
			prog_done = prog_file.read()
			prog_file.close()
		
		end_time = timeit.default_timer()

		elapsed_time = end_time - start_time

		running_total = running_total + elapsed_time
		if args.n < 10:
			print "Elapsed time: " + str(elapsed_time)

	average = running_total/args.n

	print "Average time: " + str(average)
	file_size = os.path.getsize(args.f)

	print "File size: " + str(file_size) + " bytes"

	bandwidth = file_size/average
	print "Average bandwidth: "+str(bandwidth/1024/1024) + " MiB/s"
