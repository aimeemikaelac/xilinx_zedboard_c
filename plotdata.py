#!/usr/bin/python

from pylab import plotfile, show, plot, legend, figure, semilogy
import csv

qam_file = open("qam_results_ubuntu.csv")
qam_tb_file = open("qam_testbench_results_ubuntu.csv")
qam_reader = csv.DictReader(qam_file, fieldnames=["seconds","samples","throughput","throughput_msps"])
qam_tb_reader = csv.DictReader(qam_tb_file, fieldnames=["seconds","samples","throughput","throughput_msps"])

qam_file_android = open("qam_results_android.csv")
qam_tb_file_android = open("qam_testbench_results_android.csv")
qam_reader_android = csv.DictReader(qam_file_android, fieldnames=["seconds","samples","throughput","throughput_msps"])
qam_tb_reader_android = csv.DictReader(qam_tb_file_android, fieldnames=["seconds","samples","throughput","throughput_msps"])

aes_file = open("aes_openssl_results.csv")
aes_fabric_file = open("aes_fabric_results.csv")
aes_reader = csv.DictReader(aes_file, fieldnames=["seconds","samples"])
aes_fabric_reader = csv.DictReader(aes_fabric_file, fieldnames=["seconds","samples"])

aes_file_android = open("java_aes_results.csv")
aes_fabric_file_android= open("java_aes_fabric_results.csv")
aes_reader_android = csv.DictReader(aes_file_android, fieldnames=["seconds","samples"])
aes_fabric_reader_android = csv.DictReader(aes_fabric_file_android, fieldnames=["seconds","samples"])

aes_file_android_shell = open("aes_openssl_results_android_shell.csv")
aes_fabric_file_android_shell = open("aes_fabric_results_android_shell.csv")
aes_fabric_file_android_direct = open("aes_fabric_results_aes.csv")
aes_reader_android_shell = csv.DictReader(aes_file_android_shell, fieldnames=["seconds","samples"])
aes_fabric_reader_android_shell = csv.DictReader(aes_fabric_file_android_shell, fieldnames=["seconds","samples"])
aes_fabric_reader_android_direct = csv.DictReader(aes_fabric_file_android_direct, fieldnames=["seconds","samples"])

qam_seconds = []
qam_samples = []
qam_tb_samples = []
qam_tb_seconds = []

qam_seconds_android = []
qam_samples_android = []
qam_tb_samples_android = []
qam_tb_seconds_android = []

aes_seconds = []
aes_encryptions = []
aes_fabric_seconds = []
aes_fabric_encryptions = []

aes_seconds_android = []
aes_encryptions_android = []
aes_fabric_seconds_android = []
aes_fabric_encryptions_android = []

aes_seconds_android_shell = []
aes_encryptions_android_shell = []
aes_fabric_seconds_android_shell = []
aes_fabric_encryptions_android_shell = []
aes_fabric_seconds_android_direct = []
aes_fabric_encryptions_android_direct = []

for row in qam_reader:
    qam_seconds.append(row['seconds'])
    qam_samples.append(row['samples'])

for row in qam_tb_reader:
    qam_tb_seconds.append(row['seconds'])
    qam_tb_samples.append(row['samples'])

for row in qam_reader_android:
    qam_seconds_android.append(row['seconds'])
    qam_samples_android.append(row['samples'])

for row in qam_tb_reader_android:
    qam_tb_seconds_android.append(row['seconds'])
    qam_tb_samples_android.append(row['samples'])

for row in aes_reader:
    aes_seconds.append(row['seconds'])
    aes_encryptions.append(row['samples'])

for row in aes_fabric_reader:
    aes_fabric_seconds.append(row['seconds'])
    aes_fabric_encryptions.append(row['samples'])

for row in aes_reader_android:
    aes_seconds_android.append(row['seconds'])
    aes_encryptions_android.append(row['samples'])

for row in aes_fabric_reader_android:
    aes_fabric_seconds_android.append(row['seconds'])
    aes_fabric_encryptions_android.append(row['samples'])

for row in aes_reader_android_shell:
    aes_seconds_android_shell.append(row['seconds'])
    aes_encryptions_android_shell.append(row['samples'])

for row in aes_fabric_reader_android_shell:
    aes_fabric_seconds_android_shell.append(row['seconds'])
    aes_fabric_encryptions_android_shell.append(row['samples'])

for row in aes_fabric_reader_android_direct:
    aes_fabric_seconds_android_direct.append(row['seconds'])
    aes_fabric_encryptions_android_direct.append(row['samples'])

semilogy(qam_samples, qam_seconds, '.', qam_tb_samples, qam_tb_seconds, '.')
#legend(["QAM Fabric Results - Ubuntu", "QAM Testbench Results - Ubuntu"], loc=4)
#figure()
semilogy(qam_samples_android, qam_seconds_android, '*', qam_tb_samples_android, qam_tb_seconds_android, '*')
legend(["QAM Fabric Results - Ubuntu", "QAM Testbench Results - Ubuntu","QAM Fabric Results - Android", "QAM Testbench Results - Android"], loc=4)
#plot(qam_samples, qam_seconds, '.', qam_tb_samples, qam_tb_seconds, '.')

figure()

semilogy(aes_fabric_encryptions, aes_fabric_seconds, '.', aes_encryptions, aes_seconds, '.')
semilogy(aes_fabric_encryptions_android, aes_fabric_seconds_android, '.', aes_encryptions_android, aes_seconds_android, '.')
semilogy(aes_fabric_encryptions_android_shell, aes_fabric_seconds_android_shell, '*', aes_encryptions_android_shell, aes_seconds_android_shell, '*')
semilogy(aes_fabric_encryptions_android_direct, aes_fabric_seconds_android_direct, '*')
#plot(aes_seconds, aes_encryptions, '.', aes_fabric_seconds, aes_fabric_encryptions, '.')
legend(["AES Fabric Results - Ubuntu","AES OpenSSL Results - Ubuntu", "AES Fabric Results - Android", 
"AES Android Implementation Results", "Android Fabric - Shell", "Android OpenSSL -Shell",
"AES Android Fabric Direct"], loc=4)

show()
