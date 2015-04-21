##Working Linux UIO:
* git@github.com:analogdevicesinc/linux.git
  * commit 9d40f61d01f9996d21727eef7a21845b6e73f715
    * modify devicetree with kernel
    * kernel version: 3.14.0
    * for Zedboard
    * use xilinx_zynq_defconfig
  * commit 6f5c7dacc2f28b04914f104fc1b581e0cabc2d41
    * modify devicetree with kernel
    * kernel version: 3.16.0
    * for Zedboard
    * use xilinx_zynq_defconfig
  * commit 6fea4f8b1196d67d12bc58e84ee6c8c309d1a793
    * modify devicetree with kernel
    * kernel version: 3.18.0
    * for Zedboard
    * use xilinx_zynq_defconfig

##Confirmed NOT working with default generic-uio compatible driver string:
  * git@github.com:analogdevicesinc/linux.git
    * commit d6aae494b2771f6b8f9c9cffbf3563e4585f1906
    * kernel version: 3.18.0
    * for Zedboard
    * using xilinx_zynq_defconfig


##To get working with commits after: 6fea4f8b1196d67d12bc58e84ee6c8c309d1a793
For some reason, a Xilinx engineer removed the "generic-uio" string as a valid compatible
string for the UIO driver in commit 7ebd62dbc727ef343b07c01c852a15fc4d9cc9e5. This ignores
the fact that Xilinx drivers for HLS devices depend on generic UIO in order to function and
is not a hack, and that HLS devices used from Linux do not have any Xilinx documentation from
Xilinx. In order to get around this, an argument must be passed to the kernel:
  * `uio_pdrv_genirq.of_id="generic-uio"`
This is only documented in the commit message for this commit, available at https://github.com/analogdevicesinc/linux/commit/7ebd62dbc727ef343b07c01c852a15fc4d9cc9e5#diff-fa2d4a96ed915ce5bb10cb3ec5520462.


##Notes:
* Get kernel version with: `make kernelversion`
* Seems to work with these UIO config options in the kernel:

\# CONFIG_HID_HUION is not set

CONFIG_UIO=y

\# CONFIG_UIO_CIF is not set

CONFIG_UIO_PDRV_GENIRQ=y

\# CONFIG_UIO_DMEM_GENIRQ is not set

\# CONFIG_UIO_AEC is not set

\# CONFIG_UIO_SERCOS3 is not set

\# CONFIG_UIO_PCI_GENERIC is not set

\# CONFIG_UIO_NETX is not set

\# CONFIG_UIO_MF624 is not set

CONFIG_UIO_XILINX_APM=y
