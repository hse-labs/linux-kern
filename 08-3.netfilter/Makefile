obj-m += netfilter.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod netfilter.ko

unload:
	rmmod netfilter.ko

reload:
	rmmod netfilter.ko
	insmod netfilter.ko
