TARGET_DIR=/tmp/xbm_script
CROSS_COMPILE=arm-linux-gnueabihf-
CC=gcc

ifndef KDIR
	$(error KDIR undefined)
endif
ifndef HOST
	$(error HOST undefined)
endif

SOURCE_FILES = \
			   src/evdev_manip.h \
			   src/fb_manip.h \
			   src/icons.h \
			   src/jp_hi_ka_10.h \
			   src/jp_hi_ka_12.h \
			   src/jp_hi_ka_14.h \
			   src/jp_hi_ka_ranges.h \
			   src/layouts.h \
			   src/main.c


.PHONY: all clean xbm_script deploy deploy_device_tree
all: build

build: src/jp_hi_ka_ranges.h 4x4matrix.dtbo
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o keyboard.o $(SOURCE_FILES)

src/jp_hi_ka_ranges.h:
	$(MAKE) gen_xbm_headers
gen_xbm_headers: $(TARGET_DIR)
	TARGET_DIR=$(TARGET_DIR) scripts/xbm.sh 2>/dev/null
	cp $(TARGET_DIR)/jp_hi_ka* src/

4x4matrix.dtbo:
	$(KDIR)/scripts/dtc/dtc -W no-unit_address_vs_reg -I dts -O dtb -o 4x4matrix.dtbo src/dts/4x4matrix.dts

deploy:
	scp keyboard.o $(HOST):

deploy_device_tree: 4x4matrix.dtbo
	scp 4x4matrix.dtbo $(HOST):
	ssh $(HOST) sudo -S cp ./4x4matrix.dtbo /boot/overlays/
	ssh $(HOST) sudo -S /usr/bin/dtoverlay sainsmart18 speed=2000000 fps=60 rotate=270
	ssh $(HOST) sudo -S /usr/bin/dtoverlay 4x4matrix

clean:
	-@rm -rf $(TARGET_DIR)

$(TARGET_DIR):
	@mkdir -p $@
