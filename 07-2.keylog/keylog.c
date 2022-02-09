#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <asm/io.h>

#define MODULE_NAME "keylog"

#define I8042_K_IRQ 1

#define I8042_DATA_REG 0x60
#define I8042_STATUS_REG 0x64

#define RELEASED_MASK 0x80

static int irq = I8042_K_IRQ, my_dev_id, irq_counter = 0;

static u8 i8042_read_data(void) {
	u8 val;
	val = inb(I8042_DATA_REG);
	return val;       
}

static int is_key_pressed(u8 scancode) {
	return !(scancode & RELEASED_MASK);
}

static int scancode2ascii(u8 scancode) {
	static char *row1 = "1234567890";
	static char *row2 = "qwertyuiop";
	static char *row3 = "asdfghjkl";
	static char *row4 = "zxcvbnm";

	scancode &= ~RELEASED_MASK;
	if (scancode >= 0x02 && scancode <= 0x0b)
		return *(row1 + scancode - 0x02);
	if (scancode >= 0x10 && scancode <= 0x19)
		return *(row2 + scancode - 0x10);
	if (scancode >= 0x1e && scancode <= 0x26)
		return *(row3 + scancode - 0x1e);
	if (scancode >= 0x2c && scancode <= 0x32)
		return *(row4 + scancode - 0x2c);
	if (scancode == 0x39)
		return ' ';
	if (scancode == 0x1c)
		return '\n';
	return '?';
}

static irqreturn_t my_interrupt( int irq, void *dev_id ) {
	irq_counter++;
	u8 scancode = 0;
	scancode = i8042_read_data();
	if (is_key_pressed(scancode))
		printk( KERN_INFO "07.1 - In the ISR: counter = %d, scancode: %d, char: %c pressed\n", \
				irq_counter,scancode,scancode2ascii(scancode) );
	else
		printk( KERN_INFO "07.1 - In the ISR: counter = %d, scancode: %d, char: %c released\n", \
				irq_counter,scancode - RELEASED_MASK, scancode2ascii(scancode) );
	return IRQ_NONE; 
}

static int __init my_init( void ) {
	if ( request_irq( irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id ) )
		return -1;
	printk( KERN_INFO "07.1 - Successfully loading ISR handler on IRQ %d\n", irq );
	if (request_region(I8042_DATA_REG,1,MODULE_NAME) == NULL)
		printk( KERN_INFO "07.1 - Cannot register I/O port region 0x%x\n",I8042_DATA_REG);
	else
		printk( KERN_INFO "07.1 - I/O Port region 0x%x registered\n", I8042_DATA_REG);
	if (request_region(I8042_STATUS_REG,1,MODULE_NAME) == NULL)
		printk( KERN_INFO "07.1 - Cannot register I/O port region 0x%x\n",I8042_STATUS_REG);
	else
		printk( KERN_INFO "07.1 - I/O Port region 0x%x registered\n",I8042_STATUS_REG);
	return 0;
}

static void __exit my_exit( void ) {
	synchronize_irq( irq );
	free_irq( irq, &my_dev_id );
	release_region(I8042_DATA_REG,1);
	release_region(I8042_STATUS_REG,1);
	printk( KERN_INFO "07.1 - Successfully unloading, irq_counter = %d\n", irq_counter );
}
module_init( my_init );
module_exit( my_exit );
MODULE_LICENSE( "GPL v2" );
