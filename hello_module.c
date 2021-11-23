int init_module(void) {
	printk(KERN_INFO "Hello world loadding");
}
int cleanup_module(void) {
	printk(KERN_INFO "Hello world cleanup");
}
MODULE_LICENSE("GPL");
