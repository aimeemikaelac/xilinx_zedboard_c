#!/usr/bin/python

from pylab import plotfile, show, plot, legend, figure, semilogy
import csv

qam_file = open("qam_results.csv")
qam_tb_file = open("qam_testbench_results.csv")
qam_reader = csv.DictReader(qam_file, fieldnames=["seconds","samples","throughput","throughput_msps"])
qam_tb_reader = csv.DictReader(qam_tb_file, fieldnames=["seconds","samples","throughput","throughput_msps"])

aes_file = open("aes_openssl_results.csv")
aes_fabric_file = open("aes_fabric_results.csv")
aes_reader = csv.DictReader(aes_file, fieldnames=["seconds","samples"])
aes_fabric_reader = csv.DictReader(aes_fabric_file, fieldnames=["seconds","samples"])

qam_seconds = []
qam_samples = []
qam_tb_samples = []
qam_tb_seconds = []

aes_seconds = []
aes_encryptions = []
aes_fabric_seconds = []
aes_fabric_encryptions = []

for row in qam_reader:
    qam_seconds.append(row['seconds'])
    qam_samples.append(row['samples'])

for row in qam_tb_reader:
    qam_tb_seconds.append(row['seconds'])
    qam_tb_samples.append(row['samples'])

for row in aes_reader:
    aes_seconds.append(row['seconds'])
    aes_encryptions.append(row['samples'])

for row in aes_fabric_reader:
    aes_fabric_seconds.append(row['seconds'])
    aes_fabric_encryptions.append(row['samples'])

semilogy(qam_samples, qam_seconds, '.', qam_tb_samples, qam_tb_seconds, '.')
#plot(qam_samples, qam_seconds, '.', qam_tb_samples, qam_tb_seconds, '.')
legend(["QAM Fabric Results", "QAM Testbench Results"], loc=4)

figure()

semilogy(aes_fabric_encryptions, aes_fabric_seconds, '.', aes_encryptions, aes_seconds, '.')
#plot(aes_seconds, aes_encryptions, '.', aes_fabric_seconds, aes_fabric_encryptions, '.')
legend(["AES Fabric Results","AES OpenSSL Results"], loc=4)

show()
