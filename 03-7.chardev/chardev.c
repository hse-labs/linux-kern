#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include "common.h"



struct handl_data {          // приватные данные дескриптора
	char buf[BUFSIZE+1]; // буфер данных
	int odd;             // признак начала чтения
};

static int myopen(struct inode *i, struct file *f)
{
	struct handl_data *data;
	f->private_data = kmalloc(sizeof(struct handl_data), GFP_KERNEL);
	if( NULL == f->private_data ) {
		printk( KERN_INFO, "memory allocation error\n" );
		return -ENOMEM;
	}
	data = (struct handl_data*)f->private_data;
	strcpy( data->buf, "not initialized!" );
	data->odd = 0;	
        printk(KERN_INFO "03-7.chardev... Openning device\n");
        return 0;
}


static int myrelease(struct inode *i, struct file *f)
{
	kfree (f->private_data);
        printk(KERN_INFO "03-7.chardev... Releasing device\n");
        return 0;
}


static ssize_t myread( struct file *f, char *buf, size_t count, loff_t *pos ) {
	struct handl_data* data = (struct handl_data*)f->private_data;
	printk( KERN_INFO "read - file: %p, read from %p bytes %d; refcount: %d\n",\
		f, data, count, module_refcount( THIS_MODULE ) );
	if( 0 == data->odd ) {
		int res = copy_to_user( (void*)buf, data->buf, strlen( data->buf ) );
		data->odd = 1;
		put_user( '\n', buf + strlen( data->buf ) );
		res = strlen( data->buf ) + 1;
		printk(KERN_INFO "return bytes : %d\n", res );
		return res;
	}
	data->odd = 0;
	printk( KERN_INFO "return : EOF\n" );
	return 0;
}

static ssize_t mywrite( struct file *f, const char *buf, size_t count, loff_t *pos ) {
	int res, len = count < BUFSIZE ? count : BUFSIZE;
	struct handl_data* data = (struct handl_data*)f->private_data;
	printk(KERN_INFO "write - file: %p, write to %p bytes %d; refcount: %d\n", \
		f, data, count, module_refcount( THIS_MODULE ) );
	res = copy_from_user( data->buf, (void*)buf, len );
	if( '\n' == data->buf[ len -1 ] ) data->buf[ len -1 ] = '\0';
	else data->buf[ len ] = '\0';
	printk(KERN_INFO "put bytes : %d\n", len );
	return len;
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
