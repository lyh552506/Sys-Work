ifneq ($(KERNELRELEASE),) 
	module-objs := hellokernel.o 
	obj-m := hellokernel.o 
else
    KERNELDIR := /lib/modules/$(shell uname -r)/build
    PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif	
clean:
	rm -rf module.*
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o *.ko
