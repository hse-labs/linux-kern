#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include "print/log.h"

MODULE_LICENSE( "GPL" );

static int __init my_init(void) {
	print_kernel_log("10-2 - Load mm2 module.");
	return 0;
}

static void __exit my_exit(void) {
	print_kernel_log("10-2 - Unload mm2 module.");
	return;
}

module_init(my_init);
module_exit(my_exit);

