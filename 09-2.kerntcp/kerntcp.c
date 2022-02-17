#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/fs.h>
#include <net/sock.h>

MODULE_LICENSE("GPL");

#define MY_TCP_PORT	50000
#define LISTEN_BACKLOG	5  // размер очереди входящих соединений

static struct socket *sock;	// прослушивающий (серверный) сокет
static struct socket *new_sock; // сокет для принятого соединения
struct task_struct *thread;	// task_struct для описания потока

char *testmessage = "Test message\n";

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
	struct sockaddr_in raddr;	// адрес пира
	// выполняемся пока не будет вызван kthread_stop()
	while (! kthread_should_stop()) {
		sock_create_lite(PF_INET, SOCK_STREAM, IPPROTO_TCP, &new_sock);
		new_sock->ops = sock->ops;
		// неблокирующий прием нового соединения
		err = sock->ops->accept(sock,new_sock, O_NONBLOCK, true);
		if (err < 0) {
			sock_release(new_sock);
			msleep(100); 	// пассивное ожидание 100 мс
			continue;	// снова создаем сокет
		}
		sock->ops->getname(new_sock, (struct sockaddr *) &raddr, 1);
		printk(KERN_INFO "09-2 - Connection established to %pI4:%d\n", \
			&raddr.sin_addr.s_addr, ntohs(raddr.sin_port));
		my_tcp_msgsend(new_sock, testmessage);
		sock_release(new_sock);
	}
	do_exit(0); // функция выхода из потока

}

int __init my_tcp_sock_init(void)
{
	int err;
	struct sockaddr_in addr = {  // прослушиваемый адрес
		.sin_family	= AF_INET,
		.sin_port	= htons(MY_TCP_PORT),
		.sin_addr	= { htonl(INADDR_LOOPBACK) }
	};


	int addrlen = sizeof(addr);
	err = sock_create_kern(&init_net, PF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
	if (err < 0) {
		printk(KERN_WARNING "09-2 - can't create socket\n");
		goto out;
	}

	err = sock->ops->bind(sock, (struct sockaddr *) &addr, addrlen);
	if (err < 0) {
		printk(KERN_WARNING "09-2 - can't bind socket\n");
		goto out_release;
	}

	err = sock->ops->listen(sock, LISTEN_BACKLOG); 
	if (err < 0) {
		printk(KERN_WARNING "09-2 - can't listen on socket\n");
		goto out_release;
	}
	printk(KERN_INFO "09-2 - Listening on socket.\n");
	// создание и запуск потока
	thread = kthread_run (thr_accept_sock, NULL, "%skthread%d", "kt_", 0 );

	return 0;

out_release:
	sock_release(sock);
out:
	return err;
}

void __exit my_tcp_sock_exit(void) {
	kthread_stop(thread); // устанавливает флаг завершения потока и дожидается его завершения
	if (sock != NULL)
		sock_release(sock);
	return;
}

module_init(my_tcp_sock_init);
module_exit(my_tcp_sock_exit);
