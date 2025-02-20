
![Realtek-logo](https://logos-download.com/wp-content/uploads/2016/05/Realtek_logo_logotype.png)

* [中文版](./README_CN.md)

For any questions, please visit Realtek official website [RealMCU](https://www.realmcu.com/) to get more information.

Table of Content
=================

* [1 Getting Started](#1-getting-started)
  * [Knowledge about AmebaD Demo Board](#knowledge-about-amebad-demo-board)
* [2 Documentation](#2-documentation)
* [3 Environment Setup](#3-environment-setup)
  * [Windows](#windows)
  * [Linux](#linux)
  * [Connection to Log Console](#connection-to-log-console)
* [4 Building the First Project on Ameba-D](#4-building-the-first-project-on-ameba-d)
   * [Windows GCC](#windows-gcc)
   * [Linux Terminal](#linux-terminal)
* [5 Downloading Images to Ameba-D](#5-downloading-images-to-ameba-d)
* [6 Release Notes](#6-release-notes)
* [7 Matter Support](#7-matter-support)


# 1 Getting Started

## Knowledge about AmebaD Demo Board

> For Ameba-D, there are many types of chipsets available, such as RTL8720CS, RTL8721CSM, RTL8722CSM, RTL8720DN, RTL8720DM, RTL8721DM, and RTL8722DM. 
> In addition, the chipsets can be embedded on Ameba-D DEV demo board, which is extended to various I/O interfaces. The corresponding HDK (Hardware Development Kit) documents are available, please contact RTK for further details. 

The hardware block diagram of Ameba-D demo board is shown below. 

   - USB TO UART: power supply and log print. 
   - LOG UART: the baudrate is 115200bps 
   - SWD: SWD interface, used to download images and debug with IAR. 
   - Reset button: reset Ameba-D to run firmware after IAR completes download.

<p align="center">
	<img src="https://www.amebaiot.com/wp-content/uploads/2020/02/guide_d_3.png" width="600" />
</p>

# 2 Documentation

Documentation for latest version: [RTL872xCS/RTL872xD Series](https://www.realmcu.com/en/Home/DownloadList/a6248c82-7c9a-4f79-bebf-0df240726c4d)

# 3 Environment Setup 

## Toolchain
	
Toolchains for windows and linux are supported for now.

## Windows

> Note: 
   On Windows, you can use ``Cygwin`` as the GCC development environment. ``Cygwin`` is a large collection of GNU and open source tools which provide functionality similar to a Linux distribution on Windows.

> Click [http://cygwin.com](http://cygwin.com/) and download the ``Cygwin`` package [setup-x86.exe](http://cygwin.com/setup-x86_64.exe) for your Windows platform. 

1. 32-bit ``Cygwin`` is supported both for 32-bit Windows and 64-bit Windows.
> Note: If you can not installed 32-bit Cygwin, refer to [Q: How can I install the last Cygwin version for an old, unsupported Windows?](https://cygwin.com/install.html). 
Installation at CMD:
```
setup-2.912.x86.exe --allow-unsupported-windows --site http://ctm.crouchingtigerhiddenfruitbat.org/pub/cygwin/circa/2022/11/23/063457
```
2. During the installation of ``Cygwin`` package, include `Devel -> make` and `Math -> bc` utilities on the Select Packages page, as below shows.

	<p align="center">
	  <img src="https://www.amebaiot.com/wp-content/uploads/2020/02/guide_d_1.png" width="400" />
	  <img src="https://www.amebaiot.com/wp-content/uploads/2020/02/guide_d_2.png" width="400" />
	</p>

## Linux

> Note:
   For Linux, please refer to [Ameba-D Application Note(PDF)](https://www.realmcu.com/en/Home/DownloadList/a6248c82-7c9a-4f79-bebf-0df240726c4d) to build the GCC development environment.  

## Connection to Log Console

> On AmebaD board, FTDI Chip and FT232 can be used for the log console and debugger. To view the log console, make use of the terminal tool, such as ``SecureCRT``/``TeraTerm``/``Putty`` and etc. 

1) Select the corresponding serial uart configure communicate parameter and then open it. The default baudrate is 115200bps.
2) Press the Reset button on Ameba-D board. Some messages can be found in the terminal.

	```
	12:27:31.549  calibration_ok:[2:19:11] 
	12:27:31.656  interface 0 is initialized
	12:27:31.658  interface 1 is initialized
	12:27:31.659  
	12:27:31.660  Initializing WIFI ...
	12:27:31.757  WIFI initialized
	12:27:31.758  
	12:27:31.760  init_thread(58), Available heap 0x292c0
	```

# 4 Building the First Project on Ameba-D

## Windows GCC

The following steps are for first-time developer to build GCC project, under existing RTK SDK. 

> **Building Code** This section illustrates how to build SDK. 
> First, you need to switch to GCC project directory. For Windows, open ``Cygwin`` terminal and use ``cd`` command to change directory to KM0 or KM4 project directory of Ameba-D SDK. 

> Note:
   You need to replace the ``{path}`` to your own SDK location, and add ``cygdrive`` prefix in front of the SDK location, so that Cygwin can access your file system.

```bash
cd /cygdrive/{path}/project/realtek_amebaD_va0_example/GCC-RELEASE/project_lp
cd /cygdrive/{path}/project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp
```
To build SDK for normal image, simply use ``make all`` command under the corresponding project directories on Cygwin (Windows). 

For KM0 project, if the terminal contains ``km0_image2_all.bin`` and ``Image manipulating end`` output message, it means that the image has been built successfully, as below shows.

<p align = "center">    
	<img src="https://www.amebaiot.com/wp-content/uploads/2020/02/guide_d_5.png" width="500" />
</p>

> If somehow it is built failed, type ``make clean`` to clean and then redo the make procedure. 

> After successfully built, the image file is located in ``project/realtek_amebaD_va0_example/GCC-RELEASE/project_lp/asdk/image`` , as below shows.

<p align = "center">    
	<img src="https://www.amebaiot.com/wp-content/uploads/2020/02/guide_d_6.png" width="500" />
</p>

For KM4 project, if the terminal contains ``km4_image2_all.bin`` and ``Image manipulating end`` output message, it means that the image has been built successfully, as below shows.

<p align = "center">    
	<img src="https://www.amebaiot.com/wp-content/uploads/2020/02/guide_d_7.png" width="500" />
</p>

> If somehow it built failed, type ``make clean`` to clean and then redo the make procedure. 

> After built successfully, the image file is located in ``project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp/asdk/image``, as below shows.

<p align = "center">    
	<img src="https://www.amebaiot.com/wp-content/uploads/2020/02/guide_d_8.png" width="500" />
</p>

## Linux Terminal

To build SDK for normal image on Linux, 

1. Set directory to KM0 or KM4 project by entering the following commands in Linux Terminal 

```bash
cd /{path}/project/realtek_amebaD_va0_example/GCC-RELEASE/project_lp
cd /{path}/project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp
```

2. Under the corresponding directories, execute``make all`` to build images. You may start with ``project_lp`` first followed by ``project_hp``.
   - **KM0 project (project_lp)**
     - For KM0 project, if the terminal contains ``km0_image2_all.bin`` and ``Image manipulating end`` output message, it means that the image has been built successfully.
     > If somehow it is built failed, type ``make clean`` to clean and then redo the make procedure. 

     > After successfully built, the image file is located in ``project/realtek_amebaD_va0_example/GCC-RELEASE/project_lp/asdk/image``.
   
   - **KM4 project (project_hp)**
     - For KM4 project, if the terminal contains ``km4_image2_all.bin`` and ``Image manipulating end`` output message, it means that the image has been built successfully.
     > If somehow it built failed, type ``make clean`` to clean and then redo the make procedure. 

     > After built successfully, the image file is located in ``project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp/asdk/image``, as below shows.

	<p align = "center">    
		<img src="https://www.amebaiot.com/wp-content/uploads/2020/02/guide_d_8.png" width="500" />
	</p>

> Note:
> If you encountered any error message like: 
  
```bash
make[1]: execvp: /{path}/ambd_sdk/project/realtek_amebaD_va0_example/GCC-RELEASE/project_lp/asdk/gnu_utility/prepend_header.sh: **Permission denied**
Makefile:198: recipe for target 'linker_loader' failed
```
 
> Please run the following command to grant the execution permission for the all corresponding files:

```bash 
chmod -R 777 {path}/project/realtek_amebaD_va0_example/GCC-RELEASE/
``` 


# 5 Downloading Images to Ameba-D

Realtek provides an [ImageTool](tools/AmebaD/Image_Tool/ImageTool.exe) to download images on windows.

* Environment Requirements: EX. WinXP, Win 7 Above, Microsoft .NET Framework 3.5

Assuming that the ImageTool on PC is a server, it sends images files to Ameba (client) through UART. To download image from server to client, the client must enter uart download first. 

1) **Enter into UART_DOWNLOAD mode.**

   * Push the UART DOWNLOAD button and keep it pressed.
   * Re-power on the board or press the Reset button.
   * Release the UART DOWNLOAD button.
   
   Now, Ameba board gets into UART_DOWNLOAD mode and is ready to receive data. 
   
2) **Click Chip Select(in red) on UI and select chip AmebaD.**

3) **Select the corresponding serial port and transmission baud rate. The default baudrate is 1.5Mbps (recommended).**

4) **Click the Browse button to select the images (km0_boot_all.bin/km4_boot_all.bin/km0_km4_image2.bin) to be programmed and input addresses.**

   * The image path is located in: 
     ``{path}\project\realtek_amebaD_va0_example\GCC-RELEASE\project_lp\asdk\image``
     and
     ``{path}\project\realtek_amebaD_va0_example\GCC-RELEASE\project_hp\asdk\image``,
     where {path} is the location of the project on your own computer.
   * The default target address is the SDK default image address, you can use it directly.

5) **Click Download button to start. The progress bar will show the transmit progress of each image. You can also get the message of operation successfully or errors from the log window.**

   <p align = "center">    
		<img src="https://www.amebaiot.com/wp-content/uploads/2020/02/guide_d_10.png" width="400" />
   </p>

# 6 Release Notes

> Note: 
> For Bluetooth Examples, currently we only support `BT_peripheral` , `BT_central` , `BT_scatternet` , and `BT_config` four examples.

## Vesion Sync

GitHub `ameba-rtos-d` is currently sync with 6.2 Git 73b6c5ec.

# 7 Matter Support

Please check out [ameba-rtos-matter](https://github.com/Ameba-AIoT/ameba-rtos-matter) to learn more about Matter support.
