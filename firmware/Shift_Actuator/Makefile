# SPDX-FileCopyrightText: Johannes Stockhammer
# SPDX-License-Identifier: Apache-2.0
#
# Build for the ATSAMD21G18A with arm-none-eabi-gcc.
# CMSIS_ATMEL: folder CMSIS-Atmel/CMSIS/Device/ATMEL of github.com/arduino/ArduinoModule-CMSIS-Atmel
# CMSIS_CORE:  folder CMSIS/Core/Include of github.com/ARM-software/CMSIS_5
#
#   make CMSIS_ATMEL=<path> CMSIS_CORE=<path>

CMSIS_ATMEL ?= ../../cmsis/CMSIS-Atmel/CMSIS/Device/ATMEL
CMSIS_CORE  ?= ../../cmsis/CMSIS_5/CMSIS/Core/Include

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

TARGET  = Shift_Actuator
SOURCES = $(wildcard *.c) \
          $(CMSIS_ATMEL)/samd21/source/as_gcc/startup_samd21.c \
          $(CMSIS_ATMEL)/samd21/source/system_samd21.c
LDSCRIPT = $(CMSIS_ATMEL)/samd21/source/as_gcc/samd21g18a_flash.ld

CFLAGS  = -mcpu=cortex-m0plus -mthumb -Os -std=gnu99 -Wall -Wextra \
          -ffunction-sections -fdata-sections -D__SAMD21G18A__ \
          -I. -I$(CMSIS_ATMEL) -I$(CMSIS_ATMEL)/samd21/include -I$(CMSIS_CORE) $(EXTRA_CFLAGS)
LDFLAGS = -mcpu=cortex-m0plus -mthumb -T$(LDSCRIPT) -Wl,--gc-sections \
          --specs=nano.specs --specs=nosys.specs -Wl,-Map=$(TARGET).map

all: $(TARGET).hex

$(TARGET).elf: $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	$(SIZE) $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).map

.PHONY: all clean
