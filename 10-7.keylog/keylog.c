#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>


#define MODULE_NAME "keylog"

#define I8042_K_IRQ 1


static int irq = I8042_K_IRQ, my_dev_id, irq_counter = 0;
struct kobject *kobj_ref;

static irqreturn_t my_interrupt( int irq, void *dev_id ) {
	irq_counter++;
	return IRQ_NONE; 
}

static ssize_t show( struct kobject *kobj, \
		struct kobj_attribute *attr, char *buf ) {
	sprintf( buf, "%d\n", irq_counter );
	return strlen( buf );
}	

static ssize_t store( struct kobject *kobj, \
		struct kobj_attribute *attr, const char *buf, size_t count) {
	int i, res = 0;
	const char dig[] = "0123456789";
	for( i = 0; i < count; i++ ) {
		char *p = strchr( dig, (int)buf[ i ] );
		if( NULL == p ) break;
		res = res * 10 + ( p - dig );
	}
	irq_counter = res;
	return count;
}

struct kobj_attribute i8042_attr = __ATTR(i8042_value, 0660, show, store);

static int __init my_init( void ) {
	int res = 0;
	// Создание каталога в /sys/kernel
	kobj_ref = kobject_create_and_add("i8042-ic",kernel_kobj);
	res = sysfs_create_file( kobj_ref, &i8042_attr.attr );
	if( res != 0 ) printk( KERN_ERR "10-7. bad class create file\n" );
	if ( request_irq( irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id ) )
		return -1;
	printk( KERN_INFO "10-7 - Successfully loading ISR handler on IRQ %d\n", irq );
	return 0;
}

static void __exit my_exit( void ) {
	synchronize_irq( irq );
	free_irq( irq, &my_dev_id );
	kobject_put(kobj_ref);
	sysfs_remove_file( kernel_kobj, &i8042_attr.attr );
	printk( KERN_INFO "10-7 - Successfully unloading, irq_counter = %d\n", irq_counter );
}
module_init( my_init );
module_exit( my_exit );
MODULE_LICENSE( "GPL v2" );
