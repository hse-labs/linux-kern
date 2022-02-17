#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/fs.h>
#include <net/sock.h>

MODULE_LICENSE("GPL");

#define MY_TCP_PORT		50000
#define LISTEN_BACKLOG		5

static struct socket *sock;	// прослушивающий (серверный) сокет
static struct socket *new_sock; // сокет для принятого соединения

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
		printk(KERN_WARNING "09-1 - can't create socket\n");
		goto out;
	}

	err = sock->ops->bind(sock, (struct sockaddr *) &addr, addrlen);
	if (err < 0) {
		printk(KERN_WARNING "09-1 - can't bind socket\n");
		goto out_release;
	}

	err = sock->ops->listen(sock, LISTEN_BACKLOG);
	if (err < 0) {
		printk(KERN_WARNING "09-1 - can't listen on socket\n");
		goto out_release;
	}
	printk(KERN_INFO "09-1 - Listening on socket.\n");


	return 0;

out_release:
	sock_release(sock);
out:
	return err;
}

void __exit my_tcp_sock_exit(void) {
	int err;
	struct sockaddr_in raddr;	// адрес пира
	err = sock_create_lite(PF_INET, SOCK_STREAM, IPPROTO_TCP, &new_sock);
	if (err < 0) {
		printk(KERN_WARNING "09-1 - can't create socket for accepted connection\n");
		goto out;
	}
	new_sock->ops = sock->ops;
	err = sock->ops->accept(sock,new_sock, O_NONBLOCK, true);
	if (err < 0) {
		printk(KERN_WARNING "09-1 - can't accept new connection\n");
		goto out_release_new_sock;
	}
	err = sock->ops->getname(new_sock, (struct sockaddr *) &raddr, 1);
	if (err < 0) {
		printk(KERN_WARNING "09-1 - can't get peer name\n");
		goto out_release_new_sock;
	}
	printk(KERN_INFO "09-1 - Connection established to %pI4:%d\n", \
			&raddr.sin_addr.s_addr, ntohs(raddr.sin_port));

	err = my_tcp_msgsend(new_sock, testmessage);
	if (err < 0) {
		printk(KERN_WARNING "09-1 - can't send message\n");
		goto out_release_new_sock;
	}
out_release_new_sock:
	sock_release(new_sock);
out:
	sock_release(sock);
	return;
}

module_init(my_tcp_sock_init);
module_exit(my_tcp_sock_exit);
