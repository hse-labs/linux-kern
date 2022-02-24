obj-m += print.o
print-y := main.o log.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:	
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod print.ko

unload:
	rmmod print.ko

reload:
	rmmod print.ko
	insmod print.ko
