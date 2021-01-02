#include <linux/module.h>        /* Needed by all modules */
#include <linux/kernel.h>        /* Needed for KERN_INFO  */
#include <linux/init.h>          /* Needed for the macros */
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("0.1");

#define DRIVER_AUTHOR "ido ben amram"
#define DRIVER_DESC "read and write to kernel memory"
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

/* 
 * Global variables are declared as static, so are global within the file.
 */

static int major;                /* Major number assigned to our device driver */
static int num_opens = 0;     
static struct class* kmem_class = NULL;
static struct device* kmem_device = NULL;
static char msg[BUF_LEN];        /* The msg the device will give when asked */
static char *msg_ptr;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};



static int __init chardev_init(void) {
	
	printk(KERN_INFO "kmem: Initializing the kmem LKM\n");

	// the 0 in the major number is so that the kernel dynamically returns the assigned major
	major = register_chrdev(0, DEVICE_NAME, &fops);

	if (major < 0) {
		printk(KERN_ALERT "Registering kmem failed with &d\n", major);
		return major;
	}
	// Register the device class
	kmem_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(kmem_class)) {
		class_destroy(kmem_class);
		unregister_chrdev(major, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(kmem_class);
	}

	// Register the device driver
	kmem_device = device_create(kmem_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	if (IS_ERR(kmem_device)){
		class_destroy(kmem_class);
		unregister_chrdev(major, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(kmem_class);
	}

	printk(KERN_INFO "kmem: device class created correctly\n");
	return SUCCESS;
}

static void __exit chardev_cleanup(void) {
	device_destroy(kmem_class, MKDEV(major, 0));
	class_unregister(kmem_class);
	class_destory(kmem_class);
	unregister_chrdev(major, DEVICE_NAME);
	printk(KERN_INFO "kmem: Goodbye fomr the LVM!\n");
}


static int device_open(struct inode* inode, struct file* file) {

	num_opens++;
	printk(KERN_INFO "kmem: Device has been opened %d times(s)\n", num_opens);	
	return SUCCESS;

}

static int device_release(struct inode* inode, struct file* file) {

	is_device_open--;

	module_put(THIS_MODULE);

	return 0;

}

static ssize_t device_read(struct file* filp,
			   char* buffer,
			   size_t length,
			   loff_t* offset) {

	int bytes_read = 0;

	return bytes_read;

}

static ssize_t device_write(struct file* filp, const char* buff, size_t len, loff_t* off) {
	
	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
	return -EINVAL;

}

module_init(chardev_init);
module_exit(chardev_cleanup);
