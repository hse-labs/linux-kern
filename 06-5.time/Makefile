obj-m += time.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod time.ko

unload:
	rmmod time.ko

reload:
	rmmod time.ko
	insmod time.ko
