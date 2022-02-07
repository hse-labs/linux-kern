#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>

#define SHARED_IRQ 1

static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;

module_param( irq, int, 0 );

static irqreturn_t my_interrupt( int irq, void *dev_id ) {
	irq_counter++;
	printk( KERN_INFO "06.1 - In the ISR: counter = %d\n", irq_counter );
	return IRQ_NONE; 
}

static int __init my_init( void ) {
	if ( request_irq( irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id ) )
		return -1;
	printk( KERN_INFO "06.1 - Successfully loading ISR handler on IRQ %d\n", irq );
	return 0;
}

static void __exit my_exit( void ) {
	synchronize_irq( irq );
	free_irq( irq, &my_dev_id );
	printk( KERN_INFO "06.1 - Successfully unloading, irq_counter = %d\n", irq_counter );
}
module_init( my_init );
module_exit( my_exit );
MODULE_LICENSE( "GPL v2" );
