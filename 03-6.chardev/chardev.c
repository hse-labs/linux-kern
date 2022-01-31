#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

static int minor = 0;
module_param(minor,int,0);

static char *hello_str = "ABCDEFGHJKLMNOPQRSTUVWXYZ\n";
 
static ssize_t myread(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) 
{
	int len = strlen(hello_str);
	printk(KERN_INFO "03-6.chardev... Got read %ld bytes of %u from device, ppos=%lld\n", count, len, *ppos);
	if (*ppos >= strlen(hello_str)) {
		printk( KERN_INFO "1.chardev... EOF, ppos: %lld\n", *ppos);
		return *ppos = 0;  // EOF
	}
	if (count < len) len = count; 
	int nbytes = len - copy_to_user(ubuf, hello_str + *ppos, len);
	*ppos += nbytes;
	printk(KERN_INFO "1.chardev... Read %d bytes from device, ppos=%lld\n", nbytes, *ppos);
	return nbytes;
}

static const struct file_operations file_ops =
{
	.owner = THIS_MODULE,
	.read = myread
};

static struct miscdevice misc_dev = {
	MISC_DYNAMIC_MINOR,  // выбирается автоматически
	"my_miscdev",
	&file_ops
};

static int __init init_chardev(void)
{
	if (minor !=0) misc_dev.minor = minor;
	if (misc_register( &misc_dev )) {
		printk(KERN_INFO "03-6.chardev... Error register misc device.\n");
		return -1;
	
	}
	printk(KERN_INFO "03-6.chardev... Module installed. Minor=%d\n",misc_dev.minor);
	return 0;
}
 
static void __exit cleanup_chardev(void)
{
	misc_deregister( &misc_dev);
	printk(KERN_INFO "03-6.chardev... Module unload.\n");
}

module_init(init_chardev);
module_exit(cleanup_chardev);

MODULE_LICENSE("GPL");
