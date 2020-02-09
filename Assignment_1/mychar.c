#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/version.h>
#include <linux/init.h>
#include "adcheader.h"

 
static dev_t first; // variable for device number
static struct cdev c_dev; // variable for the character device structure
static struct class *cls; // variable for the device class
static uint16_t channel=0;// bydefault channel selectred is 0
static int alignment=0;// Bydefault alignment selected is 0=right
//unsigned char cpp[100] = "heythisisastring";



/*****************************************************************************
STEP 4 as discussed in the lecture, 
my_close(), my_open(), my_read(), my_write() functions are defined here
these functions will be called for close, open, read and write system calls respectively. 
*****************************************************************************/

static int my_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Mychar : open()\n");
	return 0;
}

static int my_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Mychar : close()\n");
	return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	uint16_t adc;
	get_random_bytes(&adc,sizeof(adc));
	adc%=1023;

	if(alignment==1) // 0 defined the right alignment and 1 as left
		{ 
			adc=adc*64;
		}
     printk(KERN_INFO "CHANNEL SELECTED=%u\n,ALIGNMENT=%d\n, And random no is %i",channel,alignment,adc);

	if(copy_to_user(buf,&adc,sizeof(adc)))
	return (sizeof(adc));	// Gives the value which has not been copied
	
	return (sizeof(adc));
}


//###########################################################################################

static long my_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{	
	switch(cmd)
	{
		case IOCTL_CHANNEL_SELECTION:
			channel=arg;
			printk(KERN_INFO "Channel selected= %u",channel);
			break;
		
		case IOCTL_ALIGNMENT: 
			alignment=arg;
			printk(KERN_INFO "Alignment =%d", alignment);
			break;
	}
	return 0;
}

		
static struct file_operations fops =
{

  .owner 	= THIS_MODULE,
  .open 	= my_open,
  .release 	= my_close,
  .read 	= my_read,
  //.write 	= my_write,
 .unlocked_ioctl= my_ioctl
};
 
//########## INITIALIZATION FUNCTION ##################
// STEP 1,2 & 3 are to be executed in this function ### 
static int __init mychar_init(void) 
{
	printk(KERN_INFO "Namaste: mychar driver registered");
	
	// STEP 1 : reserve <major, minor>
	if (alloc_chrdev_region(&first, 0, 1, "BITS-PILANI") < 0)
	{
		return -1;
	}
	
	// STEP 2 : dynamically create device node in /dev directory
    if ((cls = class_create(THIS_MODULE, "chardrv")) == NULL)
	{
		unregister_chrdev_region(first, 1);
		return -1;
	}
    if (device_create(cls, NULL, first, NULL, "adc8") == NULL)
	{
		class_destroy(cls);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	
	// STEP 3 : Link fops and cdev to device node
    cdev_init(&c_dev, &fops);
    if (cdev_add(&c_dev, first, 1) == -1)
	{
		device_destroy(cls, first);
		class_destroy(cls);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	return 0;
}
 
static void __exit mychar_exit(void) 
{
	cdev_del(&c_dev);
	device_destroy(cls, first);
	class_destroy(cls);
	unregister_chrdev_region(first, 1);
	printk(KERN_INFO "Bye: mychar driver unregistered\n\n");
}
 
module_init(mychar_init);
module_exit(mychar_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Virendra Singh Chauhan <h20190580@pilani.bits-pilani.ac.in>");
MODULE_DESCRIPTION("First Assignment");
