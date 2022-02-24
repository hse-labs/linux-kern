#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE ("GPL");

#define OP_READ         0
#define OP_WRITE        1
#define OP_OOPS         OP_WRITE

static int my_oops_init (void)
{
        int *a;

        a = (int *) 0x00001234;
#if OP_OOPS == OP_WRITE
        *a = 3;
#elif OP_OOPS == OP_READ
        printk (KERN_ALERT "value = %d\n", *a);
#else
#error "Unknown op for oops!"
#endif

        return 0;
}

static void my_oops_exit (void)
{
}

module_init (my_oops_init);
module_exit (my_oops_exit);
