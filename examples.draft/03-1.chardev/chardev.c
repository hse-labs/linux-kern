#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>
 


static char *kbuf;
static dev_t first;
static unsigned int count = 1;
static int my_major = 511, my_minor = 0;
static struct cdev *my_cdev;

#define MYDEV_NAME "chardev"
#define KBUF_SIZE (size_t) ((10)* PAGE_SIZE)


//static char *procfile = "proc-status";
//module_param(procfile,charp,0660);

//static char *hello_str = "ABCDEFGHJKLMNOPQRSTUVWXYZ\n";
 
 
static ssize_t mywrite(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) 
{
	int len = count - copy_from_user(kbuf + *ppos, ubuf, count);
	*ppos += len;
	printk(KERN_INFO "1.chardev.. Wrote %d bytes to device: %s, ppos=%lld\n", len, MYDEV_NAME, *ppos);
	return len;
}
 
static ssize_t myread(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) 
{
	int len = count - copy_to_user(ubuf, kbuf + *ppos, count);
	*ppos += len;
	printk(KERN_INFO "1.chardev... Read %d bytes to device: %s, ppos=%lld\n", len, MYDEV_NAME, *ppos);
	return len;
}

static int myopen(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "1.chardev... Openning device : %s\n", MYDEV_NAME);
	return 0;
}

static int myrelease(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "1.chardev... Releasing device : %s\n", MYDEV_NAME);
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
	//ent=proc_create(procfile,0666,NULL,&myops);
	kbuf = kmalloc(KBUF_SIZE,GFP_KERNEL);
	first = MKDEV (my_major,my_minor);
	register_chrdev_region(first,count,MYDEV_NAME);
	my_cdev = cdev_alloc();
	cdev_init(my_cdev, &mycdev_ops);
	cdev_add(my_cdev,first,count);

	return 0;
}
 
static void __exit cleanup_chardev(void)
{
	//proc_remove(ent);
}

module_init(init_chardev);
module_exit(cleanup_chardev);

MODULE_LICENSE("GPL");
