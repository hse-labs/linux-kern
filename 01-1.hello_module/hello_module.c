#include <linux/module.h>

int init_module(void) {
        printk(KERN_INFO "Hello world loadding");
        return 0;
}

void cleanup_module(void) {
        printk(KERN_INFO "Hello world cleanup");
}

MODULE_LICENSE("GPL");
