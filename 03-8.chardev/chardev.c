#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include "common.h"

struct handl_data {     // приватные данные дескриптора
	int fib; 	// последнее число
	int fib_prev;   // предпоследнее число
	int num;        // п.н. числа
};

static void nullfib(struct handl_data* data) {
	data->fib = 0;
	data->fib_prev = 0;
	data->num = 0;	
}

static int myopen(struct inode *i, struct file *f)
{
	f->private_data = kmalloc(sizeof(struct handl_data), GFP_KERNEL);
	if( NULL == f->private_data ) {
		printk( KERN_INFO, "memory allocation error\n" );
		return -ENOMEM;
	}
	nullfib((struct handl_data*)f->private_data);
        printk(KERN_INFO "03-8.chardev... Openning device\n");
        return 0;
}


static int myrelease(struct inode *i, struct file *f)
{
	kfree (f->private_data);
        printk(KERN_INFO "03-8.chardev... Releasing device\n");
        return 0;
}


static ssize_t myread( struct file *f, char *buf, size_t count, loff_t *pos ) {
	struct handl_data* data = (struct handl_data*)f->private_data;
	printk( KERN_INFO "read - file: %p, read from %p bytes %d; refcount: %d\n",\
		f, data, count, module_refcount( THIS_MODULE ) );
	if( data->num < MAXFIB ) {
		int res = copy_to_user( (void*)buf, data->fib, sizeof( data->fib ) );
		data->num += 1;
		put_user( '\n', buf + sizeof( data->fib ) );
		res = sizeof( data->fib ) + 1;
		printk(KERN_INFO "return : %d\n", data->fib );
		if (data->fib == 0) data->fib = 1;
		else {
			int prev = data->fib;
			data->fib = prev + data->fib_prev;
			data->fib_prev = prev;
		}
		return res;
	}
	nullfib((struct handl_data*)f->private_data);
	printk( KERN_INFO "return : EOF\n" );
	return 0;
}

static ssize_t mywrite( struct file *f, const char *buf, size_t count, loff_t *pos ) {
	struct handl_data* data = (struct handl_data*)f->private_data;
	printk(KERN_INFO "write - file: %p, write to %p bytes %d; refcount: %d\n", \
		f, data, count, module_refcount( THIS_MODULE ) );
	data->fib_prev = 0;
	printk(KERN_INFO "put bytes : %d\n", 0 );
	return 0;
}


static const struct file_operations file_ops =
{
	.owner = THIS_MODULE,
	.read = myread,
	.write = mywrite,
	.open = myopen,
	.release = myrelease,
};

static struct miscdevice misc_dev = {
	MISC_DYNAMIC_MINOR,  // выбирается автоматически
	DEVNAME,
	&file_ops
};

static int __init init_chardev(void)
{
	if (misc_register( &misc_dev )) {
		printk(KERN_INFO "03-7.chardev... Error register misc device.\n");
		return -1;
	
	}
	printk(KERN_INFO "03-7.chardev... Module installed, minor: %d\n",misc_dev.minor);
	return 0;
}
 
static void __exit cleanup_chardev(void)
{
	misc_deregister( &misc_dev);
	printk(KERN_INFO "03-7.chardev... Module unload.\n");
}

module_init(init_chardev);
module_exit(cleanup_chardev);

MODULE_LICENSE("GPL");
