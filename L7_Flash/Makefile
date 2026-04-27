BUILD_DIR = build
SRC_DIR = Src
INC_DIR = Inc
DEFINES = -DSTM32G431xx

CFLAGS  ?= \
    -Wall -Wextra \
    -g3 -O0 \
    -I. -isystem$(INC_DIR) $(DEFINES) \
    -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

LDFLAGS ?= -Tg431.ld -nostartfiles -nostdlib --specs nano.specs -lc -lgcc -Wl,--gc-sections -Wl,-Map=$@.map

SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/startup_g4.c $(SRC_DIR)/Systick.c $(SRC_DIR)/uart.c $(SRC_DIR)/flash.c 

all: $(BUILD_DIR)/firmware.bin

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/firmware.elf: $(SOURCES) | $(BUILD_DIR)
	arm-none-eabi-gcc $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $@

$(BUILD_DIR)/firmware.bin: $(BUILD_DIR)/firmware.elf
	arm-none-eabi-objcopy -O binary $< $@

flash: $(BUILD_DIR)/firmware.bin
	st-flash --reset write $< 0x8000000

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean flashs