#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
 
static char *procfile = "proc-status";
module_param(procfile,charp,0660);
 
 
static struct proc_dir_entry *ent;
 
static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
	printk( KERN_DEBUG "LKM: 1.proc... write handler\n");
	return -1;
}
 
static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	printk( KERN_DEBUG "LKM: 1.proc... read handler\n");
	return 0;
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
