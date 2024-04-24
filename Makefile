-include config.mk
XROCK := xrock
ARM32CC := arm-none-eabi
ARMCC := aarch64-none-elf

ARMCFLAGS := -march=armv8-a -nostdlib
ARMLDFLAGS := -T Linker.ld
OBJ := src/boot.o src/mmu.o src/asm.o src/main.o src/uart.o src/timer.o src/vectors.o src/io.o src/edp.o
OBJ += src/clock.o src/soc.o src/lib.o src/bmp.o

# Boot files
XROCK_SRAM_BIN := ddr.bin
XROCK_SDRAM_BIN := os.bin

# Boot normal bare metal pinebook image
usb: $(XROCK_SRAM_BIN) $(XROCK_SDRAM_BIN)
	$(XROCK) maskrom $(XROCK_SRAM_BIN) $(XROCK_SDRAM_BIN)

makeboot.out: src/makeboot.c
	$(CC) src/makeboot.c -o makeboot.out

# Bootable SPI image
pine.img: makeboot.out $(XROCK_SRAM_BIN) $(XROCK_SDRAM_BIN)
	./makeboot.out
	echo "Burn with: sudo dd if=pine.img of=/dev/sda bs=4M conv=fsync && sync"

# Compile main binary
os.bin: $(OBJ) Linker.ld
	$(ARMCC)-ld $(OBJ) $(ARMLDFLAGS) -o src/boot.elf
	$(ARMCC)-objcopy -O binary src/boot.elf os.bin

src/ram2.o: ARMCFLAGS += -Os

DDR_OBJ := src/sram.o src/dmc.o src/io.o src/lib.o src/uart.o src/asm.o src/clock.o src/timer.o src/ram2.o
DDR_OBJ += src/vectors.o
ddr.bin: $(DDR_OBJ)
	$(ARMCC)-ld $(DDR_OBJ) -T ddr.ld -s -o src/ddr.elf
	$(ARMCC)-objcopy -O binary src/ddr.elf ddr.bin

loader.bin: os.bin src/loader.o
	$(ARMCC)-ld src/loader.o -o src/loader.elf
	$(ARMCC)-objcopy -O binary src/loader.elf loader.bin

%.o: %.c
	$(ARMCC)-gcc -MMD -c $(ARMCFLAGS) $< -o $@

%.o: %.S
	$(ARMCC)-gcc -MMD -c -D __ASM__ $(ARMCFLAGS) $< -o $@

-include $(OBJ:.o=.d)

clean:
	$(RM) src/*.o src/*.elf src/*.bin *.bin *.o *.elf blob/*.o *.img src/*.d src/*.su *.out $(OBJ)

dmesg:
	sudo dmesg -w

uart:
	sudo screen /dev/ttyUSB* 1500000

.PHONY: usb clean dmesg uart
