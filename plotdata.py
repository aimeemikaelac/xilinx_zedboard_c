#!/usr/bin/python

from pylab import plotfile, show, plot, legend, figure, semilogy, title, xlabel, ylabel, grid, savefig
import numpy
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

fig_dir = "/home/michael/android_partial_reconfiguration/publications/ieee_micro_android_partial_reconfiguration/figs/"

semilogy(qam_samples, qam_seconds, 'd', c='r', ms=5)
semilogy(qam_tb_samples, qam_tb_seconds, 's', ms=5)
legend(["Fabric Results", "Testbench Results"], loc=4, fontsize=16)
#title("QAM Benchmark - Ubuntu Linux", fontsize=20)
xlabel("Number of Samples", fontsize=18)
ylabel("Execution time (s)",fontsize=18)
grid()
#grid(which='minor', lw=.3)
savefig(fig_dir+"qam_ubuntu.pdf", bbox_inches='tight')
figure()

semilogy(qam_samples_android, qam_seconds_android, 'd', c='r', ms=5)
semilogy(qam_tb_samples_android, qam_tb_seconds_android, 's', ms=5)
legend(["Fabric Results", "Testbench Results"], loc=4, fontsize=16)
#title("QAM Benchmark - Android", fontsize=20)
xlabel("Number of Samples", fontsize=18)
ylabel("Execution time (s)", fontsize=18)
grid()
#grid(which='minor', lw=.3)
savefig(fig_dir+"qam_android.pdf",  bbox_inches='tight')
#plot(qam_samples, qam_seconds, '.', qam_tb_samples, qam_tb_seconds, '.')
figure()

semilogy(aes_fabric_encryptions, aes_fabric_seconds, 'd', c='r', ms=5)
semilogy(aes_encryptions, aes_seconds, 's', ms=5)
legend(["Fabric Results","OpenSSL Results"], loc=4, fontsize=16)
#title("AES 128-bit Benchmark - Ubuntu Linux", fontsize=20)
xlabel("Number of encryptions", fontsize=18)
ylabel("Execution time (s)", fontsize=18)
grid()
#grid(which='minor', lw=.3)
savefig(fig_dir+"aes_ubuntu.pdf",  bbox_inches='tight')
figure()

semilogy(aes_encryptions_android, aes_seconds_android, 'p', ms=5)
#semilogy(aes_fabric_encryptions_android_shell, aes_fabric_seconds_android_shell, '*')
semilogy(aes_encryptions_android_shell, aes_seconds_android_shell, 's', ms=5)
#z = numpy.polyfit(aes_encryptions_android_shell, aes_seconds_android_shell, 1)
#p = numpy.poly1d(z)
#semilogy(aes_encryptions_android_shell, p(aes_encryptions_android_shell), "r--")
semilogy(aes_fabric_encryptions_android, aes_fabric_seconds_android, 'o', ms=5)
semilogy(aes_fabric_encryptions_android_direct, aes_fabric_seconds_android_direct, 'd', ms=5)
#plot(aes_seconds, aes_encryptions, '.', aes_fabric_seconds, aes_fabric_encryptions, '.')
#legend(["Fabric Results - Application measured", 
#"AOSP AES Implementation Results", "Fabric - Command line", "OpenSSL - Command Line",
#"Fabric Results - Native measured"], loc=4)
legend(["AOSP Implementation Results",  "OpenSSL Results",
"Fabric Results - Java Measured", "FAbric Results"], loc=4, fontsize=16)
#title("AES 128-bit Benchmark - Android", fontsize=20)
xlabel("Number of encryptions", fontsize=18)
ylabel("Execution time (s)", fontsize=18)
grid()
#grid(which='minor',lw=.3)
savefig(fig_dir+"aes_android.pdf",  bbox_inches='tight')

#show()
