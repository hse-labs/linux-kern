obj-m += mm.o
mm-y := main.o log.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:	
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod mm.ko

unload:
	rmmod mm.ko

reload:
	rmmod mm.ko
	insmod mm.ko
