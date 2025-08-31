BIN=microbe.bin

ifdef BREAK
ZOS_CFLAGS += -DBREAK
endif

ifndef ZGDK_PATH
	$(error "Failure: ZGDK_PATH variable not found. It must point to ZGDK path.")
endif

GFX_COMPRESSED=1

include $(ZGDK_PATH)/base_sdcc.mk

run:
	echo "Running: " $(OUTPUT_DIR)/$(BIN)
	$(ZEAL_NATIVE_BIN) -m $(OUTPUT_DIR)/microbe.map -u $(OUTPUT_DIR)/$(BIN)

native: all run