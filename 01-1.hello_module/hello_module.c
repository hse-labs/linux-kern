#include <linux/module.h>

int init_module(void) {
        printk(KERN_INFO "LKM: Hello world loadding\n");
        return 0;
}

void cleanup_module(void) {
        printk(KERN_INFO "LKM: Hello world cleanup\n");
}

MODULE_LICENSE("GPL");
