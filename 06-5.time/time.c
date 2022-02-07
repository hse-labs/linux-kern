#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>


static u32 j1, j2;
static cycles_t c1, c2;

static int __init my_init( void ) {
	j1 = jiffies;
	c1 = get_cycles();
	printk( KERN_INFO "06-5. --LOAD-- HZ=%d, jiffies=%d, sec=%d, cycles=%lld\n", HZ, j1, j1/HZ, c1);
	return 0;
}

static void __exit my_exit( void ) {
	j2 = jiffies;
	c2 = get_cycles();
	printk( KERN_INFO "06-5. -UNLOAD- HZ=%d, jiffies=%d, sec=%d, cycles=%lld\n", HZ, j1, j1/HZ, c1);
	printk( KERN_INFO "06-5. delta jiffies=%d, delta sec=%d, delta cycles=%lld, cycles/sec=%lld\n",j2-j1, (j2-j1)/HZ,c2-c1, (c2-c1)*HZ/(j2-j1));
}
module_init( my_init );
module_exit( my_exit );
MODULE_LICENSE( "GPL v2" );
