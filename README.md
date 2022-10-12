# Secure CV2X
Provides code, repos, documents, logs for the work on secure cv2x with COTS UE

The various software modules, code and other opensourced repositories are the modified versions of the respective repositories. Please clone the base version of the same and use comparison tools like BeyondCompare to get details on the changes done for each of the repository. The base version detauls of git repos or other softwares used are listed in the original-version-info.txt

## Changes:

### ARM-TF (use v2.5 from https://github.com/ARM-software/arm-trusted-firmware)
- Changed UART clock value to support both 115200 for U-boot and ARM-TF and 3M baud for Modem. The clock freq. is shared between the various UART peripherals and the clock value with possible dividers values should be valid for both 115200 and 3M. So 50M used.
- SCP size reduced by 4KB based on size of built image

### U-Boot (use v2021.01 from https://github.com/u-boot/u-boot)
- Changed code in include/configs/sunxi-common.h, arch/arm/mach-sunxi/clock_sun6i.c and board/sunxi/board.c to handle the change of UART clock from 24M to 50M
- Changes in Makefile, arch/arm/mach-sunxi/Kconfig and arch/arm/dts/sunxi-u-boot.dtsi to handle OPTEE as part of DTB 
- Changes to handle 3GB of RAM in PinePhone 1.2 with 2 DRAM banks in arch/arm/mach-sunxi/dram_sunxi_dw.c and arch/arm/include/asm/arch-sunxi/dram_sunxi_dw.h
- defconfig files to build with OPTEE (pinephone and pine64) and python blob script for OPTEE as bl32 image


