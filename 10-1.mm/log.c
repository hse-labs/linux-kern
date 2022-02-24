#include <linux/kernel.h>
#include <linux/module.h>

#include "log.h"

void print_kernel_log(const char* msg) {
	printk(KERN_INFO "%s\n", msg);
	return;
}
