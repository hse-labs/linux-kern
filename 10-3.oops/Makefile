obj-m += oops.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod oops.ko

unload:
	rmmod oops.ko

reload:
	rmmod oops.ko
	insmod oops.ko
