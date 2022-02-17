obj-m += keylog.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod keylog.ko

unload:
	rmmod keylog.ko

reload:
	rmmod keylog.ko
	insmod keylog.ko
