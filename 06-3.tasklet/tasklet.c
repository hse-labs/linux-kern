#include <linux/module.h>
#include <linux/interrupt.h>

MODULE_LICENSE( "GPL v2" );

static cycles_t cycles1, cycles2;
static u32 j1, j2;
static int context;

void tasklet_function(struct tasklet_struct *t) {
	context = in_atomic();
	j2 = jiffies;
	cycles2 = get_cycles();
	printk(KERN_INFO "06-3 - Cycles %010lld [%05d] : tasklet running in context %d\n", \
		(long long unsigned)cycles2, j2, context );
	return;
}

DECLARE_TASKLET( my_tasklet, tasklet_function );

int init_module( void ) {
	context = in_atomic();
	tasklet_setup(&my_tasklet, tasklet_function);
	j1 = jiffies;
	cycles1 = get_cycles();
	tasklet_schedule( &my_tasklet );
	printk(KERN_INFO "06-3 - Cycles %010lld [%05d] : tasklet scheduled in context %d\n", \
		(long long unsigned)cycles1, j1, context );
	return 0;
}

void cleanup_module( void ) {
	tasklet_kill( &my_tasklet );
	return;
}
