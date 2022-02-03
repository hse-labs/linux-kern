#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

static void test_kmalloc(void) {
	size_t i;
	for (i=1; i < 0xFFFFFFFF; i=i<<1) {
		char * ptr;
		ptr = kmalloc(i, GFP_KERNEL);
		if (! ptr) {
			printk(KERN_INFO "kmalloc: unable to allocate %ld bytes\n",i);
			return;
		} else {
			//printk(KERN_INFO "kmalloc: allocated %ld bytes\n",i);
			kfree(ptr);
		}
	}
	return;
}

static void test_vmalloc(void) {
	unsigned long i;
	for (i=1; i < 0xFFFFFFFF; i=i<<1) {
		char * ptr;
		ptr = vmalloc(i);
		if (! ptr) {
			printk(KERN_INFO "vmalloc: unable to allocate %ld bytes\n",i);
			return;
		} else {
			//printk(KERN_INFO "vmalloc: allocated %ld bytes\n",i);
			vfree(ptr);
		}
	}
	return;
}

static int __init mod_init( void ) {
	printk(KERN_INFO "05-1.kalloc... Module load.\n");
        test_kmalloc();
	test_vmalloc();
        return 0;
}
static void __exit mod_cleanup(void) {
	printk(KERN_INFO "05-1.kalloc... Module unload.\n");
}
module_init(mod_init);
module_exit(mod_cleanup); 
MODULE_LICENSE( "GPL" );
