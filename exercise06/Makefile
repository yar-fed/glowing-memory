CC_TYPE ?= gcc
TARGET ?= stm32/f4
MKFILEDIR = $(dir $(abspath $(firstword $(MAKEFILE_LIST))))
OPENCM3_DIR = $(MKFILEDIR)libopencm3
PROJECT = exercise06_gl-bc
CFILES = \
		src/main.c \
		libopencm3-drivers/helper.c \
		libopencm3-drivers/keys.c
OBJS = $(CFILES:.c=.o)

CFLAGS          += -Os -ggdb3
CFLAGS          += -std=c99
CPPFLAGS        += -MD
LDFLAGS         += -static -nostartfiles
LDLIBS          += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

DEVICE=stm32f407vg

include $(OPENCM3_DIR)/mk/genlink-config.mk
include $(OPENCM3_DIR)/mk/$(CC_TYPE)-config.mk

.PHONY: lib lib-clean clean firmware all

all: lib firmware

lib:
	$(MAKE) -C $(OPENCM3_DIR) TARGETS=$(TARGET)

lib-clean:
	$(MAKE) -C $(OPENCM3_DIR) TARGETS=$(TARGET) clean

firmware: bin/$(PROJECT).elf bin/$(PROJECT).bin bin/$(PROJECT).hex

flash: |firmware
	@echo "\\033[1;37;42m--- Flashing bin/$(PROJECT).bin to device ---\\033[0m"
	$(Q)st-flash --reset --format binary write bin/$(PROJECT).bin 0x8000000

clean: lib-clean
	$(Q)$(RM) -rf bin/$(PROJECT).*
	$(Q)$(RM) -rf $(OBJS) $(OBJS:.o=.d)
	$(Q)$(RM) -rf generated.*.ld

include $(OPENCM3_DIR)/mk/genlink-rules.mk
include $(OPENCM3_DIR)/mk/$(CC_TYPE)-rules.mk
