#ifndef CHARDEV_H
#define CHARDEV_H

#include <linux/ioctl.h> // header file for ioctl

#define DEVICE_NUM 51 // Major number is defined

#define IOCTL_CHANNEL_SELECTION _IOR(DEVICE_NUM, 0, int*) //selection of channel for adc

#define IOCTL_ALIGNMENT _IOR(DEVICE_NUM, 1, char*) //selection of alignment for adc

#define FILE_NAME "/dev/adc8" //defined mydevice 

#endif
