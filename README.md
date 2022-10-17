# Secure CV2X
Provides code, repos, documents, logs for the work on Secure CV2X with COTS UE

The various software modules, code and other opensourced repositories are the modified versions of the respective repositories. Please clone the base version of the same and use comparison tools like BeyondCompare to get details on the changes done for each of the repository. The base version detauls of git repos or other softwares used are listed in the original-version-info.txt

## Hardware Setup

### Basic Setup with Server on Internet
1. To get OPTEE running on Pine64 LTS and Pinephone v1.2b refer to PINE_OPTEE_DOCUMENTATION.txt
2. Setup server on a x86 system with server.py under server_implementation. (Make sure to change the IP address to the public exposed IP and also update this information in cv2x.c in optee_os for the Trusted application)
3. Run the CV2X TA as described in PINE_OPTEE_DOCUMENTATION.txt. We use SixFab SIMs(AT&T Twilio LTE Network) for our testing

### Private Network Setup
1. The setup on the Phone or Pine64-LTS would be the same. The server now instead of on the internet would be on a private LTE network setup using USRP B200 and OAI open source repos. The Phone will use a custom SIM for the purpose
2. Basic setup information of using OAI source code and getting custom SIM card can be found here [Open Cells Setup and SIM](https://open-cells.com/index.php/2019/09/22/all-in-one-openairinterface/)

Additional detailed information to be uploaded. Instruction document is under compilation and code & artificats are being cleaned up before pushing to GIT.


## Simulation

- Install NS3 v3.26 (refer https://www.nsnam.org/wiki/Installation) or if your system has the required pre-requisites use ns3 from ns3_ref.tar.gz at [Shared_Code_Repos](https://drive.google.com/drive/folders/1X_CWSf_n6cSoHf7syK6s91MTEmlCzaOg?usp=sharing)
- Execute dir_create.py under results folder in ns3_ref/ns3-3.26/results using python3
- Open terminal and run the lte-mec simulation in ns3_ref/ns3-ref/
  > CXX=g++-8 ./waf --run lte-mec
- The resul xmls are stored under results/xmls/<BW>
- To get graph of the latency and packet loss for a particular bandwidth of downlink, set BW value in file_parser.py and run the script with python3 (Install an required packages that might be required based on the imports in the script)

Note: Replace the g++-8 with the respective version of g++ compiler on your system, we have tested the working with g++ 6,7 and 8. To view changes in the internal LTE and UDP code of the ns3 repo, compare ns-3.26 and ns-3-dev-git-ns-3.26 directories inside ns3_ref using tools like Beyond Compare, etc.


## Changes:

### ARM-TF (use v2.5 from https://github.com/ARM-software/arm-trusted-firmware)
- Changed UART clock value to support both 115200 for U-boot and ARM-TF and 3M baud for Modem. The clock freq. is shared between the various UART peripherals and the clock value with possible dividers values should be valid for both 115200 and 3M. So 50M used.
- SCP size reduced by 4KB based on size of built image

### U-Boot (use v2021.01 from https://github.com/u-boot/u-boot)
- Changed code in include/configs/sunxi-common.h, arch/arm/mach-sunxi/clock_sun6i.c and board/sunxi/board.c to handle the change of UART clock from 24M to 50M
- Changes in Makefile, arch/arm/mach-sunxi/Kconfig and arch/arm/dts/sunxi-u-boot.dtsi to handle OPTEE as part of DTB 
- Changes to handle 3GB of RAM in PinePhone 1.2 with 2 DRAM banks in arch/arm/mach-sunxi/dram_sunxi_dw.c and arch/arm/include/asm/arch-sunxi/dram_sunxi_dw.h
- defconfig files to build with OPTEE (pinephone and pine64) and python blob script for OPTEE as bl32 image

### OPTEE 
- OS
  - Information to be updated
- Client: No change in optee client source code)
- Examples: CV2X TA and Host application source code

