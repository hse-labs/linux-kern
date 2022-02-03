#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/list.h>

static char *procfile = "fib";
module_param(procfile,charp,0664);

static char *hello_str = "ABCDEFGHJKLMNOPQRSTUVWXYZ\n";
 
struct list_head fib_header;
struct fib_struct {
	int fib;
	struct list_head list;
};	


static struct proc_dir_entry *ent;
 
static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
	printk( KERN_DEBUG "05-3.proc... write handler\n");
	return -1;
}
 
static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	printk( KERN_DEBUG "05-3.proc... read request for %lu bytes, ppos: %lld\n", count, *ppos);
	//int len = strlen(hello_str);
	int len = 0;
	//if (count < len)
	//	return 0; // не можем прочитать за раз - пока в сад
	if ( *ppos != 0) {
		printk( KERN_DEBUG "05-3.proc... EOF, ppos: %lld\n", *ppos);
		*ppos = 0;
		return 0; // EOF
	}
	struct fib_struct *item, *last_item, *pre_last_item;
	struct list_head *iter;
	item = kmalloc(sizeof(*item), GFP_KERNEL);
	if (!item) {
		printk( KERN_DEBUG "05-3.proc. Cannot allocate memory\n");
		return 0;
	}
	if (list_empty( &fib_header )) {
		item->fib = 0;
	} else {
		last_item = list_entry( fib_header.prev, struct fib_struct, list );
		if (last_item->fib == 0 ) item->fib = 1;
		else {
			pre_last_item = list_entry(last_item->list.prev, struct fib_struct, list);
			item->fib = last_item->fib + pre_last_item->fib;
		}
	}
	list_add_tail(&(item->list), &fib_header);
	//printk( KERN_DEBUG "05-3.proc... Next number %u\n", item->fib);
	list_for_each(iter, &fib_header) {
		item = list_entry(iter, struct fib_struct, list);
		printk(KERN_INFO "[LIST] %d\n", item->fib);
	}
	//if(copy_to_user(ubuf,hello_str,len))
	//	return -EINVAL;
	*ppos = len;
	printk( KERN_DEBUG "05-3.proc... read return %u bytes, ppos: %lld\n", len, *ppos);
	return len;
}

static struct proc_ops myops =
{
	.proc_read = myread,
	.proc_write = mywrite,
};

int init_module(void)
{
	ent=proc_create(procfile,0666,NULL,&myops);
	INIT_LIST_HEAD(&fib_header);
	return 0;
}
 
void cleanup_module(void)
{
	struct fib_struct *item;
	struct list_head *iter, *iter_safe;
	list_for_each_safe(iter, iter_safe, &fib_header) {
		item = list_entry(iter, struct fib_struct, list);
		list_del(iter);
		kfree(item);
	}
	proc_remove(ent);
}
 
MODULE_LICENSE("GPL");
