obj-m += proc.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod proc.ko

unload:
	rmmod proc.ko

reload:
	rmmod proc.ko
	insmod proc.ko
