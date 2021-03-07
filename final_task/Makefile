TARGET_DIR=/tmp/xbm_script

.PHONY: all clean xbm_script
all: gen_xbm_headers

gen_xbm_headers: $(TARGET_DIR)
	TARGET_DIR=$(TARGET_DIR) scripts/xbm.sh
	cp $(TARGET_DIR)/jp_hi_ka* src/

clean:
	rm -rf $(TARGET_DIR)

$(TARGET_DIR):
	mkdir $@
