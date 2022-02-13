#include <linux/module.h>
#include <linux/netdevice.h>


static int __init my_init( void ) {
	struct net_device *dev;
	printk( KERN_INFO "08-2 -  module loaded at 0x%p\n", my_init );
	dev = first_net_device( &init_net );
	printk( KERN_INFO "08-2 -  dev_base address=0x%p\n", dev );
	while ( dev ) {
		printk( KERN_INFO "name = %6s irq=%4d mtu=%12u type=%12u\n", \
			dev->name, dev->irq, dev->mtu, dev->type );
		dev = next_net_device( dev );
	}
	return 0;
}

static void __exit my_exit( void ) {
	printk( KERN_INFO "08-2 - Unloading list interfaces module\n" );
}

module_init( my_init );
module_exit( my_exit );
MODULE_LICENSE( "GPL v2" );
