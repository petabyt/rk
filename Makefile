-include config.mk
# Needed to compile targets of different architectures
convert_target_arm64 = $(patsubst %.o,%.arm64.o,$1)

XROCK ?= xrock
ARMCC ?= aarch64-linux-gnu

ARMCFLAGS := -march=armv8-a -nostdlib -Wall -Wno-array-bounds -Isrc -Isrc/rk3399 -Isrc/rk3588
ARMLDFLAGS := -T Linker.ld

all: makeboot.out pinebook.bin pinebook-ddr.bin opi5.bin genbook.bin rk3588-ddr.bin demo.bin

usb3399: pinebook-ddr.bin pinebook.bin
	$(XROCK) maskrom pinebook-ddr.bin pinebook.bin

usb3588: img/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin os3588.bin
	$(XROCK) maskrom img/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin os3588.bin --rc4-off

makeboot.out: tools/makeboot.c
	$(CC) tools/makeboot.c -o makeboot.out

# Bootable Pinebook Pro SPI image
pbp.img: makeboot.out
	./makeboot.out
	echo "Burn with: sudo dd if=pine.img of=/dev/sdX bs=4M conv=fsync"

PINEBOOK_DDR_OBJ := src/rk3399/sram.o src/rk3399/ddr.o src/rk3399/io.o src/rk3399/gpio.o src/lib.o src/uart.o src/asm.o src/rk3399/clock.o src/rk3399/timer.o src/rk3399/ram2.o src/vectors.o
PINEBOOK_DDR_OBJ := $(call convert_target_arm64,$(PINEBOOK_DDR_OBJ))
$(call convert_target_arm64,src/rk3399/ram2.o): ARMCFLAGS += -Os
pinebook-ddr.bin: $(PINEBOOK_DDR_OBJ)
	$(ARMCC)-ld $(PINEBOOK_DDR_OBJ) -T ddr.ld -s -o src/ddr.elf
	$(ARMCC)-objcopy -O binary src/ddr.elf pinebook-ddr.bin

RK3588_DDR_OBJ := $(call convert_target_arm64,src/rk3588/ddr.o)
rk3588-ddr.bin: $(RK3588_DDR_OBJ)
	$(ARMCC)-ld $(RK3588_DDR_OBJ) -o src/rk3588-ddr.elf
	$(ARMCC)-objcopy -O binary src/rk3588-ddr.elf rk3588-ddr.bin

3399_OBJ := src/boot.o src/mmu.o src/rk3399/ttbl.o src/asm.o src/uart.o src/rk3399/timer.o src/vectors.o src/rk3399/gpio.o src/rk3399/edp.o src/rk3399/vop.o src/firmware.o
3399_OBJ += src/rk3399/clock.o src/rk3399/soc.o src/lib.o src/ohci.o src/i2c.o src/rk3399/mmc.o src/rk3399/io.o
PINEBOOK_OBJ := $(3399_OBJ) src/pinebook.o
PINEBOOK_OBJ := $(call convert_target_arm64,$(PINEBOOK_OBJ))
pinebook.bin: $(PINEBOOK_OBJ) Linker.ld
	$(ARMCC)-ld $(PINEBOOK_OBJ) $(ARMLDFLAGS) -o src/boot.elf
	$(ARMCC)-objcopy -O binary src/boot.elf pinebook.bin

3588_OBJ := src/boot.o src/rk3588/io.o src/uart.o src/asm.o src/vectors.o src/mmu.o src/lib.o
OPI5_OBJ := $(3588_OBJ) src/opi5.o
OPI5_OBJ := $(call convert_target_arm64,$(OPI5_OBJ))
opi5.bin: $(OPI5_OBJ) Linker.ld
	$(ARMCC)-ld $(OPI5_OBJ) $(ARMLDFLAGS) -o src/boot.elf
	$(ARMCC)-objcopy -O binary src/boot.elf opi5.bin

GENBOOK_OBJ := $(3588_OBJ) src/genbook.o
GENBOOK_OBJ := $(call convert_target_arm64,$(GENBOOK_OBJ))
genbook.bin: $(GENBOOK_OBJ) Linker.ld
	$(ARMCC)-ld $(GENBOOK_OBJ) $(ARMLDFLAGS) -o src/boot.elf
	$(ARMCC)-objcopy -O binary src/boot.elf genbook.bin

DEMO_OBJ := demo/entry.o demo/main.o demo/bmp.o
DEMO_OBJ := $(call convert_target_arm64,$(DEMO_OBJ))

demo_pinebook.bin: $(DEMO_OBJ) pinebook.bin
	$(ARMCC)-ld $(DEMO_OBJ) -Ttext=$(shell printf '0x%X' $$(stat -c%s pinebook.bin)) -o src/boot.elf
	$(ARMCC)-objcopy -O binary src/boot.elf demo_pinebook.bin

%.o: %.c
	gcc -MMD -c $< -o $@
%.arm64.o: %.c
	$(ARMCC)-gcc -MMD -c $< $(ARMCFLAGS) -o $@
%.arm64.o: %.S
	$(ARMCC)-gcc -D __ASM__ -MMD -c $< $(ARMCFLAGS) -o $@

-include $(wildcard **/*.d)

clean:
	find src demo tools \( -name '*.d' -o -name '*.o' -o -name '*.elf' -o -name '*.bin' \) -type f -delete
	rm -rf *.bin *.elf *.out

dmesg:
	sudo dmesg -w
uart:
	sudo screen /dev/ttyUSB* 115200
uart2:
	sudo screen /dev/ttyUSB* 1500000

uartlog:
	sudo screen -L -Logfile log.txt /dev/ttyUSB* 115200
bear:
	make clean && bear -- make -j`nproc`

.PHONY: usb clean dmesg uart
