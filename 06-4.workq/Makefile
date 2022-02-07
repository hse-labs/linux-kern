obj-m += workq.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod workq.ko

unload:
	rmmod workq.ko

reload:
	rmmod workq.ko
	insmod workq.ko
