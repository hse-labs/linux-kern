obj-m += chardev.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod chardev.ko

unload:
	rmmod chardev.ko

reload:
	rmmod chardev.ko
	insmod chardev.ko
