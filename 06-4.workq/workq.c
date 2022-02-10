#include <linux/module.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>

MODULE_LICENSE( "GPL v2" );

static int works = 5; // число отложенных работ
module_param( works, int, 0 );

static struct workqueue_struct *my_wq;

typedef struct {
	int id;
	u32 j;
	cycles_t cycles;
	struct work_struct my_work;
} my_work_t;

static void my_wq_function( struct work_struct *work ) {
	u32 j = jiffies;
	cycles_t cycles = get_cycles();
	my_work_t *wrk = container_of(work,my_work_t,my_work);
	printk( "06-4 - #%d : %010lld [%05d] => %010lld [%05d] = %06lu : context %d\n", \
		wrk->id, (long long unsigned)wrk->cycles, wrk->j, \
		(long long unsigned)cycles, j, \
		(long unsigned)( cycles - wrk->cycles ), in_atomic() );
	kfree( (void *)wrk );
	return;
}

int init_module( void ) {
	int n, ret;
	if( ( my_wq = create_workqueue( "my_queue" ) ) )
		for( n = 0; n < works; n++ ) {
			my_work_t *work = (my_work_t*)kmalloc( sizeof(my_work_t), GFP_KERNEL );
			if( work ) {
				INIT_WORK( &work->my_work, my_wq_function );
				work->id = n;
				work->j = jiffies;
				work->cycles = get_cycles();
				ret = queue_work( my_wq, &work->my_work );
				if( !ret ) return -EPERM;
			}
			else return -ENOMEM;
		}
	else return -EBADRQC;
	return 0;
}

void cleanup_module( void ) {
	flush_workqueue( my_wq );
	destroy_workqueue( my_wq );
	return;
}

