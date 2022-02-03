obj-m += listhead.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod listhead.ko

unload:
	rmmod listhead.ko

reload:
	rmmod listhead.ko
	insmod listhead.ko
