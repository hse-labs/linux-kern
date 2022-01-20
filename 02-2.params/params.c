#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/string.h>

static int debug=0;
module_param(debug,int,0660);
 
static char *devname = "simpdev";
module_param(devname,charp,0660);

#define FIXLEN 5 
static char str[ FIXLEN ] = "";  
module_param_string(cparam, str, sizeof(str), 0660 ); 
	
static int aparam[] = { 0, 0, 0, 0, 0 }; 
static int arnum = sizeof( aparam ) / sizeof( aparam[ 0 ] ); 
module_param_array( aparam, int, &arnum, 0660 ); 

void print_params(void) {
	int i;
	char msg[200] = "";
	printk(KERN_INFO "LKM: 2.params... debug   = %d\n",debug);
	printk(KERN_INFO "LKM: 2.params... devname = %s\n",devname);
	printk(KERN_INFO "LKM: 2.params... cparam  = %s { %d }\n",str, strlen(str));
	sprintf( msg, "aparam [ %d ] = ", arnum ); 
   	for( i = 0; i < arnum; i++ ) 
		sprintf( msg + strlen( msg ), " %d ", aparam[ i ] ); 
   	printk( KERN_INFO "LKM: 2.params... %s\n", msg ); 
}

int init_module(void) {
	printk(KERN_INFO "LKM: 2.params loadding\n");
	if (debug > 0) {
		print_params();
	}
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "LKM: 2.params cleanup\n");
	if (debug > 0 ) {
		print_params();
	}
}

MODULE_LICENSE("GPL");
