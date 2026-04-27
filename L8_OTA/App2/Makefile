BUILD_DIR = build
SRC_DIR = Src
INC_DIR = Inc

CFLAGS  ?=  -W -Wall -Wextra -Werror -Wundef -Wshadow -Wdouble-promotion \
	-Wformat-truncation -fno-common -Wconversion \
	-g3 -O0 -ffunction-sections -fdata-sections -I. -I$(INC_DIR)\
	-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 $(EXTRA_CFLAGS)

LDFLAGS ?= -Tg431.ld -nostartfiles -nostdlib --specs nano.specs -lc -lgcc -Wl,--gc-sections -Wl,-Map=$@.map

SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/startup_g4.c $(SRC_DIR)/Systick.c 

all: $(BUILD_DIR)/firmware.bin

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/firmware.elf: $(SOURCES) | $(BUILD_DIR)
	arm-none-eabi-gcc $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $@

$(BUILD_DIR)/firmware.bin: $(BUILD_DIR)/firmware.elf
	arm-none-eabi-objcopy -O binary $< $@

flash: $(BUILD_DIR)/firmware.bin
	st-flash --reset write $< 0x08012000

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean flashs