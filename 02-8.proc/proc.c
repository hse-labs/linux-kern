#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
 
#define BUFSIZE 100

static char *procfile = "proc-status";
module_param(procfile,charp,0660);

char *hello_str = "ABCDEFGHJKLMNOPQRSTUVWXYZ\n";
static char buf[BUFSIZE];

static struct proc_dir_entry *ent;
 
static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
	//int len = strlen(hello_str);
	printk( KERN_DEBUG "LKM: 8.proc... write request for %lu bytes of %u, ppos: %lld\n", count, BUFSIZE, *ppos);
	if ( (count + *ppos) >= BUFSIZE )  // фиксированный размер буфера, столько записать не сможем
		return -EFAULT;
	if(copy_from_user(buf+*ppos,ubuf,count))
		return -EFAULT;
	*ppos += count;
	buf[*ppos]=0;
	printk( KERN_DEBUG "LKM: 8.proc... wrote data: %s, len: %ld",buf,strlen(buf));
	printk( KERN_DEBUG "LKM: 8.proc... wrote %lu bytes, ppos: %lld\n", count, *ppos);
	return count; 
}
 
static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	int len = strlen(buf);
	printk( KERN_DEBUG "LKM: 8.proc... read request for %lu bytes of %u, ppos: %lld\n", count,BUFSIZE, *ppos);
	if ( *ppos >= len) {
		printk( KERN_DEBUG "LKM: 8.proc... EOF, ppos: %lld\n", *ppos);
		return *ppos = 0; // EOF
	}
	if (count < len) len = count;
	int nbytes = len - copy_to_user(ubuf,buf+(* ppos),len);
	*ppos += nbytes;
	printk( KERN_DEBUG "LKM: 8.proc... read return %d bytes, ppos: %lld\n", nbytes, *ppos);
	return nbytes;
}

static struct proc_ops myops =
{
	.proc_read = myread,
	.proc_write = mywrite
};

int init_module(void)
{
	ent=proc_create(procfile,0666,NULL,&myops);
	buf[0]=0;
	return 0;
}
 
void cleanup_module(void)
{
	proc_remove(ent);
}
 
MODULE_LICENSE("GPL");
