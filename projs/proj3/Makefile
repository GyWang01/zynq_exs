ifneq ($(KERNELRELEASE),)
obj-m := mouse_drv.o
else
KDIR := /home/emb-zynq/kernel/

all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
