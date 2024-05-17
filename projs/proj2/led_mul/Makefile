ifneq ($(KERNELRELEASE),)
obj-m := led_gpios_mul.o
else
KDIR := /home/emb-zynq/kernel/

all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
