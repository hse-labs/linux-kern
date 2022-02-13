#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/fs.h>
#include <net/sock.h>

MODULE_LICENSE("GPL");

#define MY_TCP_PORT		60000
#define LISTEN_BACKLOG		5

static struct socket *sock;	// прослушивающий (серверный) сокет

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
		printk(KERN_WARNING "08-4 - can't create socket\n");
		goto out;
	}

	err = sock->ops->bind(sock, (struct sockaddr *) &addr, addrlen);
	if (err < 0) {
		printk(KERN_WARNING "08-4 - can't bind socket\n");
		goto out_release;
	}

	err = sock->ops->listen(sock, LISTEN_BACKLOG);
	if (err < 0) {
		printk(KERN_WARNING "08-4 - can't listen on socket\n");
		goto out_release;
	}
	printk(KERN_INFO "08-4 - Listening on socket.\n");
	return 0;

out_release:
	sock_release(sock);
out:
	return err;
}

void __exit my_tcp_sock_exit(void) {
	sock_release(sock);
	return;
}

module_init(my_tcp_sock_init);
module_exit(my_tcp_sock_exit);
