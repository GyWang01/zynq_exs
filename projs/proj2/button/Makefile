ifneq ($(KERNELRELEASE),)
obj-m := button.o
else
KDIR := /home/emb-zynq/kernel/

all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
