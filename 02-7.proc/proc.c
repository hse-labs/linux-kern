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
	printk( KERN_DEBUG "LKM: 7.proc... write request for %lu bytes, ppos: %lld\n", count, *ppos);
	char buf[BUFSIZE];
	if ( count > BUFSIZE )  // фиксированный размер буфера, столько записать не сможем
		return -EFAULT;
	if(copy_from_user(buf,ubuf,count))
		return -EFAULT;
	*ppos = count;
	buf[count]=0;
	printk( KERN_DEBUG "LKM: 7.proc... wrote data: %s\n",buf);
	printk( KERN_DEBUG "LKM: 7.proc... wrote %lu bytes, ppos: %lld\n", count, *ppos);
	return count; 
}
 
static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	int len = strlen(hello_str);
	printk( KERN_DEBUG "LKM: 7.proc... read request for %lu bytes of %u, ppos: %lld\n", count,len, *ppos);
	if ( *ppos >= len) {
		printk( KERN_DEBUG "LKM: 7.proc... EOF, ppos: %lld\n", *ppos);
		*ppos = 0;
		return 0; // EOF
	}
	if (count < len)
		len = count;
	if (copy_to_user(ubuf,hello_str+(* ppos),len))
		return -EINVAL;
	*ppos += len;
	printk( KERN_DEBUG "LKM: 7.proc... read return %d bytes, ppos: %lld\n", len, *ppos);
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
