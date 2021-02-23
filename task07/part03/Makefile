PWD = $(shell pwd)

TARGET = src/t7
obj-m   := $(TARGET).o

all: default clean

default:
	$(MAKE) -C $(BUILD_KERNEL) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules

clean:
	@rm -f *.o .*.cmd .*.flags *.mod *.mod.c *.order
	@rm -f .*.*.cmd *.symvers *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions

disclean: clean
	@rm -f *.ko
