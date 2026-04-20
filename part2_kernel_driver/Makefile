# Makefile for kernel driver
# ATU Sligo - Operating Systems Interfacing Lab 8

obj-m += memory.o

KERNEL_VERSION = $(shell uname -r)
KERNEL_DIR = /lib/modules/$(KERNEL_VERSION)/build

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

info:
	@echo "Kernel version: $(KERNEL_VERSION)"
	@echo "Kernel dir: $(KERNEL_DIR)"

.PHONY: all clean info