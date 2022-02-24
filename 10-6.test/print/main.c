#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include "log.h"

MODULE_LICENSE( "GPL" );

static int __init my_init(void) {
	print_kernel_log("10-6 - Load print module.");
	print_kernel_log2("10-6 - Load print module.");
	print_kernel_log3("10-6 - Load print module.");
	return 0;
}

static void __exit my_exit(void) {
	print_kernel_log("10-6 - Unload print module.");
	print_kernel_log2("10-6 - Unload print module.");
	print_kernel_log3("10-6 - Unload print module.");
	return;
}

module_init(my_init);
module_exit(my_exit);

