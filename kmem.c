#include <linux/module.h>        /* Needed by all modules */
#include <linux/kernel.h>        /* Needed for KERN_INFO  */
#include <linux/init.h>          /* Needed for the macros */
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>

#define DRIVER_AUTHOR "ido ben amram"
#define DRIVER_DESC "read and write to kernel memory"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("0.1");

#define SUCCESS (0)
#define DEVICE_NAME "kmem"
#define CLASS_NAME "kmem"

/*
 *  Prototypes - this would normally go in a .h file
 */
static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);
static loff_t device_llseek(struct file*, loff_t, int);

/* 
 * Global variables are declared as static, so are global within the file.
 */

static int major;                /* Major number assigned to our device driver */
static int num_opens = 0;     
static dev_t kmem_device = {0};
static struct class* kmem_class = NULL;
static struct cdev* kmem_cdev = NULL;

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.llseek = device_llseek,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};



static int __init kmem_init(void) {
	
	int err = 0;
	struct device* dev_ret = NULL;
	printk(KERN_INFO "kmem: Initializing the kmem LKM\n");

	
	major = alloc_chrdev_region(&kmem_device,
				    0, // first minor number
				    1, // count
				    DEVICE_NAME);
	if (major < 0) {
		printk(KERN_ALERT "Registering kmem failed with &d\n", major);
		return major;
	}

	// Register the device class
	kmem_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(kmem_class)) {
		unregister_chrdev_region(kmem_device, 1);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(kmem_class);
	}

	// Register the device driver
	dev_ret  = device_create(kmem_class, NULL, kmem_device, NULL, DEVICE_NAME);
	if (IS_ERR(dev_ret)){
		class_destroy(kmem_class);
		unregister_chrdev_region(kmem_device, 1);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(dev_ret);
	}

	cdev_init(kmem_cdev, &fops);
	kmem_cdev->owner = THIS_MODULE;
	kmem_cdev->ops = &fops;
	err = cdev_add(kmem_cdev, kmem_device, 1);
	if(err) {
		device_destroy(kmem_class, kmem_device);
		class_unregister(kmem_class);
		class_destroy(kmem_class);
		unregister_chrdev_region(kmem_device, 1);
		return err;
	
	}

	printk(KERN_INFO "kmem: device class created correctly\n");
	return SUCCESS;
}

static void __exit kmem_cleanup(void) {
	device_destroy(kmem_class, MKDEV(major, 0));
	class_unregister(kmem_class);
	class_destroy(kmem_class);
	unregister_chrdev_region(kmem_device, 1);
	printk(KERN_INFO "kmem: Goodbye fomr the LVM!\n");
}


static int device_open(struct inode* inode, struct file* filp) {

	num_opens++;
	printk(KERN_INFO "kmem: Device has been opened %d times(s)\n", num_opens);	
	return SUCCESS;

}

static int device_release(struct inode* inode, struct file* filp) {

	num_opens--;
	printk(KERN_INFO "kmem closed");	
	return 0;

}
static loff_t device_llseek(struct file* filp, loff_t off, int whence) {

	loff_t newpos = {0};

	switch(whence) {
		
		case 0: // SEEK_SET
			newpos = off;
			break;
		
		case 1: // SEEK_CUR
			newpos = filp->f_pos + off;
			break;

		case 2: // SEEK_END
		default:
			return -EINVAL;
	}

	if (newpos < 0)
		return -EINVAL;
	filp->f_pos = newpos;
	return newpos;

}

static ssize_t device_read(struct file* filp,
			   char* buffer,
			   size_t length,
			   loff_t* f_pos) {

	int bytes_read = 0;
	char* kern_buff = (char*)kmalloc(length, GFP_KERNEL);
	if (kern_buff == NULL)
		return -1;

	while (bytes_read < length) {
		kern_buff[bytes_read] = *((char*)f_pos);
	}
	copy_to_user(buffer, kern_buff, bytes_read);
	*f_pos += bytes_read;

	kfree(kern_buff);
	return bytes_read;

}

static ssize_t device_write(struct file* filp, const char* buff, size_t len, loff_t* off) {
	
	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
	return -EINVAL;

}

module_init(kmem_init);
module_exit(kmem_cleanup);
