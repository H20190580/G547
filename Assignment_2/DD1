#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/usb.h>
#include<linux/slab.h>


# define BOMS_RESET 0xFF
# define BOMS_RESET_TYPE 0x21

# define BOMS_GET_MAX_LUN 0xFE
# define BOMS_GET_MAX_LUN_TYPE 0xA1

#define SANDISK_PENDRIVE_VID  0x0781   // Mass Storage Device # 1
#define SANDISK_PENDRIVE_PID  0x558a

#define UMAX_PENDRIVE_VID  0x0204   // Mass Storage Device # 2
#define UMAX_PENDRIVE_PID  0x6025

# define be_to_int32(buf)(((buf)[0] << 24) | ((buf)[1] << 16) | ((buf)[2] << 8) | (buf)[3])

# define RETRY_MAX 5


// Structure of Command Block Wrapper (CBW)
struct command_status_wrapper {
  uint8_t dCSWSignature[4];
  uint32_t dCSWTag;
  uint32_t dCSWDataResidue;
  uint8_t bCSWStatus;
};


// Structure of Command Status Wrapper (CSW)
struct command_block_wrapper{
  uint8_t dCBWSignature[4];
  uint32_t dCBWTag;
  uint32_t dCBWDataTransferLength;
  uint8_t bmCBWFlags;
  uint8_t bCBWLUN;
  uint8_t bCBWCBLength;
  uint8_t CBWCB[16];
};

static uint8_t cdb_length[256] = {
//	 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  0
	06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  1
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  2
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  3
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  4
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  5
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  6
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  7
	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  8
	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  9
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  A
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  B
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  C
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  D
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  E
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  F
};


//Function Declarations:
static int get_mass_storage_status(struct usb_device * , uint8_t, uint32_t);
static int send_mass_storage_command(struct usb_device * , uint8_t, uint8_t, uint8_t * , uint8_t, int, uint32_t * );
static int test_usbstorage(struct usb_device * , struct usb_interface * , uint8_t, uint8_t);
uint8_t endpoint_in=0,endpoint_out=0;
uint32_t expected_tag;

// Recognize the Device and its corresponding information
static int usb_probe(struct usb_interface * interface, const struct usb_device_id * id) {
    int i, r;
    unsigned char epAddr, epAttr;
    struct usb_endpoint_descriptor * ep_desc;
    struct usb_device * udev;
    udev = container_of(interface -> dev.parent, struct usb_device, dev); // Pointer to device descriptor

    printk(KERN_INFO "\n");
    printk(KERN_INFO "\n");
    printk(KERN_INFO "KNOWN USB DRIVE DETECTED\n");
    printk(KERN_INFO "Pendrive Vendor ID = %#06x \n", udev -> descriptor.idVendor); // Vendor ID 
    printk(KERN_INFO "Pendrive Product ID = %#06x \n", udev -> descriptor.idProduct); // Product ID

    printk(KERN_INFO "USB DEVICE CLASS : %x", interface -> cur_altsetting -> desc.bInterfaceClass);
    printk(KERN_INFO "USB DEVICE SUB CLASS : %x", interface -> cur_altsetting -> desc.bInterfaceSubClass);
    printk(KERN_INFO "USB DEVICE Protocol : %x", interface -> cur_altsetting -> desc.bInterfaceProtocol);

    printk(KERN_INFO "No. of Endpoints = %d\n", interface -> cur_altsetting -> desc.bNumEndpoints);
    for (i = 0; i < interface -> cur_altsetting -> desc.bNumEndpoints; i++) 
    {
      ep_desc = & interface -> cur_altsetting -> endpoint[i].desc; 
      epAddr = ep_desc -> bEndpointAddress;
      epAttr = ep_desc -> bmAttributes;
      if ((epAttr & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
        if (epAddr & 0x80) {
          printk(KERN_INFO "EP %d is Bulk IN\n", i);
          endpoint_in = ep_desc -> bEndpointAddress;
        } else {
          printk(KERN_INFO "EP %d is Bulk OUT\n", i);
          endpoint_out = ep_desc -> bEndpointAddress;
        }
      }
    }

//Checking for Valid SCSI Command with Device class = 8 and Subclass = 6
    if ((interface -> cur_altsetting -> desc.bInterfaceClass == 0x08) && ((interface -> cur_altsetting -> desc.bInterfaceSubClass == 0x01) || (interface -> cur_altsetting -> desc.bInterfaceSubClass == 0x06)) && (interface -> cur_altsetting -> desc.bInterfaceProtocol == 0x50)) {
			printk(KERN_INFO "Valid SCSI mass storage device.\n");
			printk(KERN_INFO "Initiating SCSI commands\n");
			r = test_usbstorage(udev, interface, endpoint_in, endpoint_out);
      if (r < 0) {
        printk(KERN_INFO "Test_USBStorage failure, Return Value = %d\n",r);
        return -1;
      }
		}
		else{
			printk(KERN_INFO "Not a Valid SCSI mass storage device\n");
		}
      return 0;
    }



// Sending mass storage fucntion preparing cbd to CBW
static int send_mass_storage_command(struct usb_device * udev, uint8_t endpoint, uint8_t lun,
      uint8_t * cdb, uint8_t direction, int data_length, uint32_t * ret_tag) {
      static uint32_t tag = 1;
      struct command_block_wrapper * cbw = (struct command_block_wrapper * ) kmalloc(sizeof( struct command_block_wrapper), GFP_KERNEL);      
      uint8_t cdb_len;
      int i, size, r;
      if (cdb == NULL) {
        return -1;
      }
      cdb_len = cdb_length[cdb[0]];  // Fetching the size of cdb

      if ((cdb_len == 0) || (cdb_len > sizeof(cbw -> CBWCB))) {
        printk(KERN_INFO "send_mass_storage_command: don't know how to udev this command (%02X, length %d)\n",
          cdb[0], cdb_len);
        return -1;
      }
      memset(cbw, 0, sizeof(cbw));
      cbw -> dCBWSignature[0] = 'U';
      cbw -> dCBWSignature[1] = 'S';
      cbw -> dCBWSignature[2] = 'B';
      cbw -> dCBWSignature[3] = 'C';

      * ret_tag = tag;
      cbw -> dCBWTag = tag++;
      cbw -> dCBWDataTransferLength = data_length;
      cbw -> bmCBWFlags = direction;
      cbw -> bCBWLUN = lun;
      cbw -> bCBWCBLength = cdb_len;

      memcpy(cbw -> CBWCB, cdb, cdb_len);

      i = 0;
      do {
        r = usb_bulk_msg(udev, usb_sndbulkpipe(udev, endpoint), (void * ) cbw, 31, & size, 1000);

        i++;
      } while ((r != 0) && (i < RETRY_MAX));		// RETRY_MAX = 5
      
	if (r != 0) {
        printk(KERN_INFO "usb_bulk_msg function failed with code%d and no of bytes transfered is %d number of times tried is %d\n", r, size, i);
        return -1;
      }
      	printk(KERN_INFO " Successfuly sent %d CDB bytes\n", size);
      	return 0;
    }



// Testing the mass storage SCSI Commands: BOMRS Command, GET MAX LUN Command, READ CAPCity Command
static int test_usbstorage(struct usb_device * udev, struct usb_interface * interface, uint8_t endpoint_in, uint8_t endpoint_out) {
      int size, ret;
      long max_lba, block_size;
      long Pendrive_Size_in_GB;
      uint8_t cdb[16]; // SCSI Command Descriptor Block
      uint8_t * buffer = (uint8_t * ) kmalloc(64 * sizeof(uint8_t), GFP_KERNEL);
      uint8_t * lun = (uint8_t * ) kmalloc(sizeof(uint8_t), GFP_KERNEL);

	printk(KERN_INFO "\n \n");

// BULK ONLY MASS STORAGE RESET Command	
	printk(KERN_INFO "BULK ONLY MASS STORAGE RESET Status:\n");
     	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0), BOMS_RESET, BOMS_RESET_TYPE, 0, interface -> cur_altsetting -> desc.bInterfaceNumber, 0, 0, 0);
      if (ret < 0) {
        printk(KERN_INFO "BOMSR Failure, Return value = %d \n", ret);
        return -1;
      }
     printk(KERN_INFO "Reset successful \n");
	printk(KERN_INFO "\n \n");
	printk(KERN_INFO "\n \n");

// GET MAX LUN Command	
	printk(KERN_INFO "GET MAX LUN Status:\n");
	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0), BOMS_GET_MAX_LUN, BOMS_GET_MAX_LUN_TYPE, 0, interface -> cur_altsetting -> desc.bInterfaceNumber, (void * ) lun, 1, 0);
	if (ret < 0) {
	 printk(KERN_INFO "GET MAX LUN Failure, Return value = %d \n", ret);
	 return -1;
			}
			printk(KERN_INFO "MAX Logical Unit Number in the Device = %d \n", * lun);

	printk(KERN_INFO "\n \n");
	printk(KERN_INFO "\n \n");
			
// READ CAPACITY Command: opcode=25h
      printk(KERN_INFO "READ CAPACITY OF PENDRIVE:\n");
      memset(buffer, 0, sizeof(buffer));
      memset(cdb, 0, sizeof(cdb));

      cdb[0] = 0x25; // Read capacity command

      send_mass_storage_command(udev, endpoint_out, * lun, cdb, 0x80, 0x08, & expected_tag);

      ret = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, endpoint_in), buffer, 64, & size, 0);

      if (ret < 0) {
        printk(KERN_INFO "Read capacity Command Failure, Return Value = %d \n", ret);
        return -1;
      }

      max_lba = (long) be_to_int32( & buffer[0]);
      block_size = (long) be_to_int32( & buffer[4]);
      Pendrive_Size_in_GB = (((max_lba + 1) / (1024 * 1024)) * block_size) / 1024;
      printk(KERN_INFO "   Max LBA = %08li, Block Size = %08li ,Pendrive Size in GB = %li GB \n", max_lba, block_size, Pendrive_Size_in_GB);
      if (get_mass_storage_status(udev, endpoint_in, expected_tag) == -1) {
        printk(KERN_INFO "Mass Stoarage Status = FAILED");
      }
      return 0;
    }


// Reading the status of device through CSW(Command Status Wrapper)
static int get_mass_storage_status(struct usb_device * udev, uint8_t endpoint, uint32_t expected_tag) {
      int r, size;
      struct command_status_wrapper * csw = (struct command_status_wrapper * ) kmalloc(sizeof(struct command_status_wrapper), GFP_KERNEL);;

      r = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, endpoint), (void * ) csw, 31, & size, 0);// Exact size of CSW = 31 bytes

      if (r != 0) {
        printk(KERN_INFO " CSW failure, Return Value= %d\n",r);
        return -1;
      }

      if (size != 13) {
        printk(KERN_INFO " Received Size of CSW : received %d bytes (expected 13)\n", size);
        return -1;
      }
      if (csw -> dCSWTag != expected_tag) {
        printk(KERN_INFO "   get_mass_storage_status: mismatched tags (expected %08X, received %08X)\n",
          expected_tag, csw -> dCSWTag);
        return -1;
      }
      printk(KERN_INFO "   Mass Storage Status: %02X (%s)\n", csw -> bCSWStatus, csw -> bCSWStatus ? "FAILED" : "Success");
      return 0;
    }


// Disconnect Function
static void usb_disconnect(struct usb_interface * interface) {
      printk(KERN_INFO "	USB_Device_Distached \n");
      return;
    }


// Devices which can be connected(VId,PID)
static struct usb_device_id usb_table[] = {
      {USB_DEVICE(SANDISK_PENDRIVE_VID, SANDISK_PENDRIVE_PID)},
      {USB_DEVICE(UMAX_PENDRIVE_VID, UMAX_PENDRIVE_PID)},
      {}
    };


// Operation Struture
static struct usb_driver usbdev_driver = {
      name: "PENDRIVE DRIVER", //name of the device
      probe: usb_probe, // Whenever Device is plugged in
      disconnect: usb_disconnect, // When we remove a device
      id_table: usb_table, //  List of devices served by this driver
    };

static int __init device_init(void) {
      int ret;
      ret = usb_register( & usbdev_driver);
      if (ret < 0) {
        printk(KERN_NOTICE " 	USB Device connection failed,Return value = %d\n ",ret);
        return -1;
      }
      printk(KERN_NOTICE "	 UAS READ Capcaity Driver Inserted \n");
      return 0;
    }

static void __exit device_exit(void) {
      usb_deregister( & usbdev_driver);
      printk(KERN_NOTICE "USB driver removed \n");
    }

    module_init(device_init);
    module_exit(device_exit);
    MODULE_LICENSE("GPL");
