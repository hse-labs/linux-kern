obj-m += inter.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod inter.ko

unload:
	rmmod inter.ko

reload:
	rmmod inter.ko
	insmod inter.ko
