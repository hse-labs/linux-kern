#include <linux/kernel.h>
#include <linux/module.h>

#include "log.h"


void print_kernel_log(const char* msg) {
	printk(KERN_INFO "%s\n", msg);
	return;
}
EXPORT_SYMBOL( print_kernel_log );

void print_kernel_log2(const char* msg) {
	static char buf[100];
	strcpy(buf, msg);
	strcat(buf, __FUNCTION__);
	printk(KERN_INFO "%s\n", buf);
	return;
}
EXPORT_SYMBOL( print_kernel_log2 );

void print_kernel_log3(const char* msg) {
	printk(KERN_INFO "%s from %s\n", msg, __FUNCTION__);
	return;
}
EXPORT_SYMBOL( print_kernel_log3 );
