ifneq ($(KERNELRELEASE),)
obj-m := led_gpios.o
else
KDIR := /home/emb-zynq/kernel/

all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif