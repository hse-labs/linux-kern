obj-m += mm2.o print/
mm2-y := main.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:	
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod print/print.ko
	insmod mm2.ko

unload:
	rmmod mm2.ko
	rmmod print/print.ko

reload:
	rmmod mm2.ko
	rmmod print/print.ko
	insmod print/print.ko
	insmod mm2.ko
