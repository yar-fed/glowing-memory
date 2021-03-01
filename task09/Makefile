TARGET = task09

obj-m := $(TARGET).o
$(TARGET)-objs := src/t9.o

.PHONY: all clean
all:
	$(MAKE) -C $(BUILD_KERNEL) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

