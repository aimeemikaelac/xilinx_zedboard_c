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

##Notes:
* Get kernel version with: `make kernelversion`
* Seems to work with these UIO config options in the kernel:
# CONFIG_HID_HUION is not set
CONFIG_UIO=y
# CONFIG_UIO_CIF is not set
CONFIG_UIO_PDRV_GENIRQ=y
# CONFIG_UIO_DMEM_GENIRQ is not set
# CONFIG_UIO_AEC is not set
# CONFIG_UIO_SERCOS3 is not set
# CONFIG_UIO_PCI_GENERIC is not set
# CONFIG_UIO_NETX is not set
# CONFIG_UIO_MF624 is not set
CONFIG_UIO_XILINX_APM=y
