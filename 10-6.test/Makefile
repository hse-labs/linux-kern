obj-m += test.o print/
test-y := main.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:	
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod print/print.ko
	insmod test.ko

unload:
	rmmod test.ko
	rmmod print/print.ko

reload:
	rmmod test.ko
	rmmod print/print.ko
	insmod print/print.ko
	insmod test.ko
