obj-m += netinf.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod netinf.ko

unload:
	rmmod netinf.ko

reload:
	rmmod netinf.ko
	insmod netinf.ko
