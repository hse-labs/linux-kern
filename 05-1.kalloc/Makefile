obj-m += kalloc.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod kalloc.ko

unload:
	rmmod kalloc.ko

reload:
	rmmod kalloc.ko
	insmod kalloc.ko
