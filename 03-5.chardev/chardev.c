#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>

#define DEVNAME "chardev"
#define MINOR_START 0
#define MINOR_COUNT 5

static struct cdev hcdev;
static struct class *devclass;


static int major = 0; // Это значение будет, если загрузка модуля производится без параметра
module_param(major,int,0);

static char *hello_str = "ABCDEFGHJKLMNOPQRSTUVWXYZ\n";
 
static ssize_t myread(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) 
{
	int len = strlen(hello_str);
	printk(KERN_INFO "1.chardev... Got read %ld bytes of %u from device: %s, ppos=%lld\n", count, len, DEVNAME, *ppos);
	if (*ppos >= len) {
		printk( KERN_INFO "1.chardev... EOF, ppos: %lld\n", *ppos);
		return *ppos = 0;  // EOF
	}
	if (count < len) len = count; 
	int nbytes = len - copy_to_user(ubuf, hello_str + *ppos, len);
	*ppos += nbytes;
	printk(KERN_INFO "1.chardev... Read %d bytes from device: %s, ppos=%lld\n", nbytes, DEVNAME, *ppos);
	return nbytes;
}

static int device_open = 0;

static int myopen(struct inode *inode, struct file *file)
{
	if (device_open) {
		printk(KERN_INFO "1.chardev... Error - BUSY device : %s\n", DEVNAME);
		return -EBUSY;
	}
	device_open++;
	printk(KERN_INFO "1.chardev... Openning device : %s\n", DEVNAME);
	return 0;
}

static int myrelease(struct inode *inode, struct file *file)
{
	device_open--;
	printk(KERN_INFO "1.chardev... Releasing device : %s\n", DEVNAME);
	return 0;
}

static const struct file_operations mycdev_ops =
{
	.owner = THIS_MODULE,
	.read = myread,
	.open = myopen,
	.release = myrelease
};

static int __init init_chardev(void)
{
	dev_t device;
	int i;
	if (major != 0) {  // major определен пользователем через параметр загрузки модуля
		device = MKDEV (major,MINOR_START); // первое устройство диапазона
		if (register_chrdev_region(device, MINOR_COUNT, DEVNAME) < 0) { // запрашиваем регистрацию диапазона
			printk(KERN_INFO "1.chardev... Cannot register device region. Device: %s, Major: %d\n", \
					DEVNAME, major);
			return -1;
		}
	} else {
		if (alloc_chrdev_region(&device,MINOR_START,MINOR_COUNT,DEVNAME) < 0) { // все minor-номера
			printk(KERN_INFO "1.chardev... Cannot allocate device region for device: %s\n", DEVNAME);
			return -1;
		}
		major = MAJOR( device ); // фиксируем полученный свободный major
	}
	cdev_init(&hcdev, &mycdev_ops); 
	hcdev.owner = THIS_MODULE;
	if (cdev_add(&hcdev,device,MINOR_COUNT) < 0) {  // добавить надо все, начиная с первого
		unregister_chrdev_region(MKDEV(major,MINOR_START),MINOR_COUNT);
		printk(KERN_INFO "1.chardev... Cannot add char device. Device: %s, Major: %d\n", DEVNAME, major);
		return -1;
	}
	devclass = class_create( THIS_MODULE, "dyn_class");
	for (i=0; i < MINOR_COUNT; i++) {
		device = MKDEV(major, MINOR_START+i);
		device_create(devclass, NULL, device, NULL, "%s%d", DEVNAME, i);
	}
	printk(KERN_INFO "1.chardev... Module installed. Name: %s, Devices: %d[%d-%d]\n", DEVNAME, major,MINOR_START, MINOR(device));
	return 0;
}
 
static void __exit cleanup_chardev(void)
{
	dev_t device;
	int i;
	for (i=0; i < MINOR_COUNT; i++) {
		device = MKDEV( major, MINOR_START + i);
		device_destroy(devclass, device);
	}
	class_destroy(devclass);
	cdev_del(&hcdev);
	unregister_chrdev_region(MKDEV(major,MINOR_START),MINOR_COUNT);
	printk(KERN_INFO "1.chardev... Module unload. Device: %s, Major: %d\n", DEVNAME, major);
}

module_init(init_chardev);
module_exit(cleanup_chardev);

MODULE_LICENSE("GPL");
