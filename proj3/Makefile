ifneq ($(KERNELRELEASE),)
obj-m := kmpu.o
else
KDIR := /home/emb-zynq/kernel/

all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif