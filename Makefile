iobj-m += hello_module.c

all:
	make -C /lib/modules/$(uname -r)/build modules

clean:
	make -C /lib/modules/$(uname -r)/build clean

load:
	insmod hello_module.ko
	dmesg -c

unload:
	rmmod hello_module.ko
	dmesg -c
