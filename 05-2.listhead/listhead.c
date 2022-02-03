#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>
struct data {
        int n;
        struct list_head list;
};
static void test_lists(void)
{
        struct list_head *iter, *iter_safe;
        struct data *item;
        int i;
        LIST_HEAD(list);
        for (i = 0; i < 10; i++) {
                item = kmalloc(sizeof(*item), GFP_KERNEL);
                if (!item) goto out;
                item->n = i;
                list_add(&(item->list), &list);
        }
        list_for_each(iter, &list) {
                item = list_entry(iter, struct data, list);
                printk(KERN_INFO "[LIST] %d\n", item->n);
        }
out:
        list_for_each_safe(iter, iter_safe, &list) {
                item = list_entry(iter, struct data, list);
                list_del(iter);
                kfree(item);
        }
}

MODULE_LICENSE( "GPL" );
static int __init mod_init( void ) {
        test_lists();
        return 0;
}
module_init( mod_init );
