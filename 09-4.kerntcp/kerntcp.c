#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/fs.h>
#include <net/sock.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");

#define MY_TCP_PORT	50000
#define LISTEN_BACKLOG	20  // размер очереди входящих соединений
#define THREAD_NUM	20

static struct socket *sock;	// прослушивающий (серверный) сокет
unsigned int counter;		// счетчик подключений

struct mutex mtx_counter; // мьютекс для счетчика
struct mutex mtx_socket;  // мьютекс для сокета

struct thread_data {
	struct socket *sock;	   // сокет для принятого соединения
	struct task_struct *thread;// task_struct для описания потока
} threads[THREAD_NUM];


int my_tcp_msgsend(struct socket *s, char *message) {
	struct msghdr msg;
	struct kvec iov;
	int len = strlen(message) + 1;
	iov.iov_base = message;
	iov.iov_len = len;
	memset(&msg, 0, sizeof(msg));
	return kernel_sendmsg(s, &msg, &iov, 1, len);
}

int thr_accept_sock(void *data) {
	int err = 0;
	char msg[200] = "";
	struct socket *new_sock = (struct socket*)data;
	// указатель current устанавливается на значение task_struct для данного потока
	printk(KERN_INFO "09-4. In thread: %s [PID = %d]\n", current->comm, current->pid);
	// выполняемся пока не будет вызван kthread_stop()
	while (! kthread_should_stop()) {
		sock_create_lite(PF_INET, SOCK_STREAM, IPPROTO_TCP, &new_sock);
		new_sock->ops = sock->ops;
		// неблокирующий прием нового соединения
		mutex_lock(&mtx_socket);
		err = sock->ops->accept(sock,new_sock, O_NONBLOCK, true);
		mutex_unlock(&mtx_socket);
		if (err < 0) {
			sock_release(new_sock);
			msleep(1); // пассивное ожидание 100 мс
			continue;
		}
		mutex_lock(&mtx_counter);
		counter++;
		sprintf( msg, "Message #%d sended from: %s [PID = %d]\n", \
				counter, current->comm, current->pid );
		mutex_unlock(&mtx_counter);
		my_tcp_msgsend(new_sock, msg);
		printk(KERN_INFO "09-4. %s", msg);
		sock_release(new_sock);
	}
	do_exit(0); // функция выхода из потока

}

int __init my_tcp_sock_init(void)
{
	int err, i;
	struct sockaddr_in addr = {  // прослушиваемый адрес
		.sin_family	= AF_INET,
		.sin_port	= htons(MY_TCP_PORT),
		.sin_addr	= { htonl(INADDR_LOOPBACK) }
	};
	int addrlen = sizeof(addr);
	counter = 0;
	mutex_init(&mtx_counter); // инициализация мьютекса счетчика
	mutex_init(&mtx_socket);
	err = sock_create_kern(&init_net, PF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
	if (err < 0) {
		printk(KERN_WARNING "09-4 - can't create socket\n");
		goto out;
	}
	err = sock->ops->bind(sock, (struct sockaddr *) &addr, addrlen);
	if (err < 0) {
		printk(KERN_WARNING "09-4 - can't bind socket\n");
		goto out_release;
	}
	err = sock->ops->listen(sock, LISTEN_BACKLOG); 
	if (err < 0) {
		printk(KERN_WARNING "09-4 - can't listen on socket\n");
		goto out_release;
	}
	printk(KERN_INFO "09-4 - Listening on socket.\n");
	for (i=0; i < THREAD_NUM; i++) {
		threads[i].thread = kthread_run (thr_accept_sock, (void *)threads[i].sock, "%skthread%d", "kt_", i );
	}
	return 0;
out_release:
	sock_release(sock);
out:
	return err;
}

void __exit my_tcp_sock_exit(void) {
	int i;
	for (i=0; i < THREAD_NUM; i++)
		kthread_stop(threads[i].thread); // устанавливает флаг завершения потока и дожидается его завершения
	if (sock != NULL)
		sock_release(sock);
	return;
}

module_init(my_tcp_sock_init);
module_exit(my_tcp_sock_exit);
