#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>

MODULE_LICENSE("GPL");

static unsigned int my_nf_hookfn(void *priv,
		struct sk_buff *skb, const struct nf_hook_state *state) {
	struct iphdr *iph = ip_hdr(skb);
	if (iph->protocol == IPPROTO_TCP) {	// для протокола TCP
		struct tcphdr *tcph = tcp_hdr(skb);
		if (tcph->syn && !tcph->ack)  	// ловим SYN-запрос
			printk(KERN_INFO "08-3 - TCP connection initiated from "
				"%pI4:%u\n",&iph->saddr, ntohs(tcph->source));
	}
	return NF_ACCEPT;
}

static struct nf_hook_ops my_nfho = {
	.hook        = my_nf_hookfn,
	.hooknum     = NF_INET_LOCAL_OUT,  	// для исходящего трафика
	.pf          = PF_INET,
	.priority    = NF_IP_PRI_FIRST
};

int __init my_hook_init(void) {
	return nf_register_net_hook(&init_net, &my_nfho);
}

void __exit my_hook_exit(void) {
	nf_unregister_net_hook(&init_net, &my_nfho);
	return;
}

module_init(my_hook_init);
module_exit(my_hook_exit);
