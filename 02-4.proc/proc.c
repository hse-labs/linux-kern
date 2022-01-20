#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
 
#define BUFSIZE 100

static char *procfile = "proc-status";
module_param(procfile,charp,0660);

static char *hello_str = "ABCDEFGHJKLMNOPQRSTUVWXYZ\n";
 
static struct proc_dir_entry *ent;
 
static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
	printk( KERN_DEBUG "LKM: 4.proc... write handler\n");
	return -1;
}
 
static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	printk( KERN_DEBUG "LKM: 4.proc... read request for %lu bytes, ppos: %lld\n", count, *ppos);
	int len = strlen(hello_str);
	if (count < len)
		return 0; // не можем прочитать за раз - пока в сад
	if ( *ppos >= len) {
		printk( KERN_DEBUG "LKM: 4.proc... EOF, ppos: %lld\n", *ppos);
		*ppos = 0;
		return 0; // EOF
	}
	if(copy_to_user(ubuf,hello_str,len))
		return -EINVAL;
	*ppos = len;
	printk( KERN_DEBUG "LKM: 4.proc... read return %u bytes, ppos: %lld\n", len, *ppos);
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
	return 0;
}
 
void cleanup_module(void)
{
	proc_remove(ent);
}
 
MODULE_LICENSE("GPL");
