#include <linux/module.h>
#include <linux/netdevice.h>

static struct net_device *dev;


static int my_open( struct net_device *dev ) {
	printk( KERN_INFO "08-1 - Hit: my_open(%s)\n", dev->name );
	netif_start_queue( dev ); // Инициализация очереди передачи
	return 0;
}

static int my_close( struct net_device *dev ) {
	printk( KERN_INFO "08-1 - Hit: my_close(%s)\n", dev->name );
	netif_stop_queue( dev );
	return 0;
}

static int stub_start_xmit( struct sk_buff *skb, struct net_device *dev ) {
	// здесь отправляем пакет взаимодействуя с оборудованием
	dev_kfree_skb( skb );
	return 0;
}

static struct net_device_ops ndo = {
	.ndo_open = my_open,
	.ndo_stop = my_close,
	.ndo_start_xmit = stub_start_xmit,
};

static void my_setup( struct net_device *dev ) {
	int j;
	for( j = 0; j < ETH_ALEN; ++j ) // заполняем МАС
		dev->dev_addr[ j ] = (char)j;
	ether_setup( dev );
	dev->netdev_ops = &ndo;
	printk( KERN_INFO "08-1 - In own setup %s!\n", dev_name( &dev->dev ) );

}

static int __init my_init( void ) {
	printk( KERN_INFO "08-1 - Loading stub network module:...." );
	dev = alloc_netdev( 0, "stub%d", 1, my_setup );
	if( register_netdev( dev ) ) {
		printk( KERN_INFO "08-1 - Failed to register\n" );
		free_netdev( dev );
		return -1;
	}
	printk( KERN_INFO "08-1 - Succeeded in loading %s\n", dev_name( &dev->dev ) );
	return 0;
}

static void __exit my_exit( void ) {
	printk( KERN_INFO "08-1 - Unloading stub network module\n" );
	unregister_netdev( dev );
	free_netdev( dev );
}

module_init( my_init );
module_exit( my_exit );
MODULE_LICENSE( "GPL v2" );
