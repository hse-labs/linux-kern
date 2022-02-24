obj-m += run.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod run.ko

unload:
	rmmod run.ko

reload:
	rmmod run.ko
	insmod run.ko
