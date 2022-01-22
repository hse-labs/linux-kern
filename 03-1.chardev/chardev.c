#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>
 


static struct cdev hcdev;


static char *devname = "chardev";
module_param(devname,charp,0);

static int major = 255;
module_param(major,int,0);

static char *hello_str = "ABCDEFGHJKLMNOPQRSTUVWXYZ\n";
 
 
static ssize_t mywrite(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) 
{
	int len = count - copy_from_user(hello_str + *ppos, ubuf, count);
	*ppos += len;
	printk(KERN_INFO "1.chardev.. Wrote %d bytes to device: %s, ppos=%lld\n", len, devname, *ppos);
	return len;
}
 
static ssize_t myread(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) 
{
	int len = strlen(hello_str);
	printk(KERN_INFO "1.chardev... Got read %ld bytes of %u from device: %s, ppos=%lld\n", count, len, devname, *ppos);
	if (*ppos >= strlen(hello_str)) {
		printk( KERN_INFO "1.chardev... EOF, ppos: %lld\n", *ppos);
		return *ppos = 0;  // EOF
	}
	if (count < len) len = count; 
	int nbytes = len - copy_to_user(ubuf, hello_str + *ppos, len);
	*ppos += nbytes;
	printk(KERN_INFO "1.chardev... Read %d bytes from device: %s, ppos=%lld\n", nbytes, devname, *ppos);
	return nbytes;
}

static int device_open = 0;

static int myopen(struct inode *inode, struct file *file)
{
	if (device_open) {
		printk(KERN_INFO "1.chardev... Error - BUSY device : %s\n", devname);
		return -EBUSY;
	}
	device_open++;
	printk(KERN_INFO "1.chardev... Openning device : %s\n", devname);
	return 0;
}

static int myrelease(struct inode *inode, struct file *file)
{
	device_open--;
	printk(KERN_INFO "1.chardev... Releasing device : %s\n", devname);
	return 0;
}

static const struct file_operations mycdev_ops =
{
	.owner = THIS_MODULE,
	.read = myread,
	.write = mywrite,
	.open = myopen,
	.release = myrelease
};

static int __init init_chardev(void)
{
	dev_t device;
	device = MKDEV (major,0);
	if (register_chrdev_region(device,1,devname) < 0) {
		printk(KERN_INFO "1.chardev... Cannot register device region. Device: %s, Major: %d\n", devname, major);
		return -1;
	}
	cdev_init(&hcdev, &mycdev_ops); 
	hcdev.owner = THIS_MODULE;
	if (cdev_add(&hcdev,device,1) < 0) {
		unregister_chrdev_region(MKDEV(major,0),1);
		printk(KERN_INFO "1.chardev... Cannot add char device. Device: %s, Major: %d\n", devname, major);
		return -1;
	
	}

	printk(KERN_INFO "1.chardev... Module installed. Device: %s, Major: %d\n", devname, major);
	return 0;
}
 
static void __exit cleanup_chardev(void)
{
	cdev_del(&hcdev);
	unregister_chrdev_region(MKDEV(major,0),1);
	printk(KERN_INFO "1.chardev... Module unload. Device: %s, Major: %d\n", devname, major);
}

module_init(init_chardev);
module_exit(cleanup_chardev);

MODULE_LICENSE("GPL");