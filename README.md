# A Real Time Object Detection Security System With Automated Response For Closed Spaces

### Team SkyEye

### Chenjie(Lavi) Zhao (lavizhao@bu.edu), Yaying Zheng (yyzheng@bu.edu), Junwei Zhou (junwei23@bu.edu)  


This project is to design a real time security surveillance system using object detection algorithms. This design will be equipped with its own automated defense system to immediately neutralize suspicious targets. The system has several components including bluetooth, webcam, Gumstix microprocessor, computer, hardware and hardware control.
Read for project report for more detailed information.
Below are explanations on how to run the software end of all the modules in this project.

Syntax  
`# - command line in target gumstix kernel(minicom emulation terminal)`    
$ - command line in host computer terminals         
// or /* */- comments  
M: - message displayed (if needed)  

----------------------------------------------------------------------------------------------------------------------------
##### Object Detection (/source/ObjectDetection)
/* Tensorflow MobileNet + SSD(Single Shot MultiBox Detector) model pre-trained with COCO dataset is used  
Python3, pip and opencv should be installed before running the program  
Modify video device number in ODC.py if needed  
Modify class_id to detect different objects, default is class_id = 1 for detecting human */  
$ python3 odc.py  

----------------------------------------------------------------------------------------------------------------------------
##### Motor and Trigger Control (/source/Motor)
// Motors and trigger are controlled by the GPIO outputs of the gumstix  
// Open minicom  
$ minicom  
// In another terminal, source into host containing kernel if kernel is not installed in local   
// environment  
$ source /ad/eng/courses/ec/ec535/bashrc_ec535  
// Run the Makefile for the kernel module  
$ make  
// Load .ko file on to Gumstix by using CTRL-A, S in minicom to choose zmodem  
// Once loaded, insert module using the following commands  
`# mknod /dev/motor c 61 0`
`# insmod motor.ko`

----------------------------------------------------------------------------------------------------------------------------
##### Bluetooth (/source/Bluetooth)
// If you are using Ubuntu, install libbluetooth-dev using following commands  
$ sudo apt-get install libbluetooth-dev  
// Source into host containing kernel if kernel is not installed in local environment  
$ source /ad/eng/courses/ec/ec535/bashrc_ec535  
// Cross-Compile readInstr.c and link bluetooth library  
// Compile sendInstr.c using gcc and link bluetooth library  
$ gcc sendInstr.c -lbluetooth -o sendInstr  
$ arm-linux-gcc readInstr.c -lbluetooth -o readInstr  
// Open minicom in another terminal if not already open  
$ minicom  
// In minicom, use CTRL-A, S to choose zmodem and load readInstr binary executable  
// Once loaded, run readInstr followed by sendInstr  
`# ./readInstr`
$ ./sendInstr  

----------------------------------------------------------------------------------------------------------------------------
##### Webcam UVC Driver + User-level Application to Capture Images (/source/Webcam)
// Load 5 kernel modules in /source/webcam/driver onto gumstix  
// v4l2 api setup on a 32-bit environment  
`# insmod compat_ioctl32.ko`
`# insmod v4l1-compat.ko`
`# insmod v4l2-common.ko`

// webcam device setup, depends on previous kernel modules  
`# insmod videodev.ko `
M: Linux video capture interface: v2.00                                              

// UVC Device driver setup, depends on previous kernel modules  
`# insmod uvcvideo.ko`
M: usbcore: registered new interface driver uvcvideo                                
M: USB Video Class driver (SVN r238)  

// when webcam is plugged in  
M: usb 1-2: new full speed USB device using pxa27x-o3  
M: usb 1-2: configuration #1 chosen from 1 choice                                  
M: usbcore: registered new interface driver snd-usb-audio  

/* CROSS COMPILE ALL DEPENDENCIES from source code because no admin access to the lab computer kernel and jpeglib is not included in the kernel  
download the Unix format jpeglib source code package first from http://www.ijg.org/  
CC and host decides what cross compiler to use to build the library  
The path declared in prefix is the path to the folder where you want built libraries to be in */  
$ ./configure CC=arm-linux-gcc --prefix=/source/webcam/jpeg-build --host=arm-linux  
$ make  
$ make install  

// Cross compile webcam application and statically link the built jpeglib library  
$ arm-linux-gcc uvccapture.c v4l2uvc.c -ljpeg -static -L/source/webcam/jpeg-build/lib -I/source/webcam/jpeg-build/include -std=gnu99 -O2 -DLINUX -DVERSION=\"0.4\" -o uvccapture  

// Transfer the executable uvccapture to the gumstix  
`# ./uvccapture`

// v4l2grab application tested on Ubuntu 18.04  
// Must specify image file name  
$ gcc v4l2grab.c yuv.c -o v4l2grab  
$ ./v4l2grab image.jpg  

Some helper commands  
`# lsusb`
`# cat /dev/video*`
`# ls -ltrh /dev/video*`
`# dmesg | less`
