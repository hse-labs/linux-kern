#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define MEMSIZ	100

static int __init hello_init( void ) {
	void* Addr;
	Addr = kmalloc(MEMSIZ, GFP_KERNEL);
	printk( KERN_INFO "10-5. Address: %p\n", Addr );
	kfree(Addr);
	return -1;
}
module_init( hello_init );
