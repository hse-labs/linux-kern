#include <linux/module.h>
#include <linux/moduleparam.h>

static int debug=0;
module_param(debug,int,0660);
 
static char *devname = "simpdev";
module_param(devname,charp,0660);

int init_module(void) {
	printk(KERN_INFO "LKM: 1.params loadding\n");
	printk(KERN_INFO "LKM: 1.params... name=%s debug=%d\n",devname,debug);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "LKM: 1.params cleanup\n");
	printk(KERN_INFO "LKM: 1.params... name=%s debug=%d\n",devname,debug);
}

MODULE_LICENSE("GPL");
