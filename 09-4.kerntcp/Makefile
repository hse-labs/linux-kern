obj-m += kerntcp.o

KDIR := /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	insmod kerntcp.ko

unload:
	rmmod kerntcp.ko

reload:
	rmmod kerntcp.ko
	insmod kerntcp.ko
