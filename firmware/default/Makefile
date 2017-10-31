###############################################################################
# Makefile for the project LightingUPS
###############################################################################

## General Flags
PROJECT = LightingUPS
MCU = attiny84
F_CPU = 1000000
TARGET = LightingUPS.elf
CC = avr-gcc.exe

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -DF_CPU=$(F_CPU)UL
CFLAGS += -Wall -gstabs  -O3 -fsigned-char -fshort-enums -std=gnu99
CFLAGS += -Wa,-adhlns=$(<:.c=.lst)
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map,LightingUPS.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"..\CommonCode" -I"C:\WinAVR-20100110\avr\include" -I"C:\WinAVR-20100110\avr\bin" -I".." 

## Objects that must be built in order to link
OBJECTS = LightingUPS.o SystemTime.o ADCManager.o DataHistory.o \
        CommandProcessor.o EEPROMStorage.o SystemMode.o Console.o \
	BatteryMonitor.o PhotocellMonitor.o PushbuttonMonitor.o \
        MainsMonitor.o MotionMonitor.o InternalTemperatureMonitor.o \
        PowerCommand.o PowerSwitches.o StatusIndicators.o \
	ByteQueue.o SoftwareSerialTx.o SoftwareSerialRx.o CharString.o StringUtils.o \
        EEPROM.o \
        RamSentinel.o

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) LightingUPS.hex LightingUPS.eep size

## Compile
LightingUPS.o: ../LightingUPS.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

SystemTime.o: ../SystemTime.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ADCManager.o: ../CommonCode/ADCManager.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

DataHistory.o: ../CommonCode/DataHistory.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ByteQueue.o: ../CommonCode/ByteQueue.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

SoftwareSerialTx.o: ../CommonCode/SoftwareSerialTx.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

SoftwareSerialRx.o: ../CommonCode/SoftwareSerialRx.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

EEPROM.o: ../CommonCode/EEPROM.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

CharString.o: ../CommonCode/CharString.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

RamSentinel.o: ../CommonCode/RamSentinel.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

StringUtils.o: ../CommonCode/StringUtils.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

BatteryMonitor.o: ../BatteryMonitor.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

CommandProcessor.o: ../CommandProcessor.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

EEPROMStorage.o: ../EEPROMStorage.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

SystemMode.o: ../SystemMode.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Console.o: ../Console.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

PhotocellMonitor.o: ../PhotocellMonitor.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

PushbuttonMonitor.o: ../PushbuttonMonitor.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

MainsMonitor.o: ../MainsMonitor.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

MotionMonitor.o: ../MotionMonitor.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

InternalTemperatureMonitor.o: ../InternalTemperatureMonitor.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

PowerCommand.o: ../PowerCommand.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

PowerSwitches.o: ../PowerSwitches.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

StatusIndicators.o: ../StatusIndicators.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) LightingUPS.elf dep/* LightingUPS.hex LightingUPS.eep

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

