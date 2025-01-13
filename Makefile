-include config.mk
XROCK := xrock
ARMCC := aarch64-none-elf

ARMCFLAGS := -march=armv8-a -nostdlib -Wall -Wno-array-bounds -Isrc
ARMLDFLAGS := -T Linker.ld

usb3399: ddr.bin os3399.bin
	$(XROCK) maskrom ddr.bin os3399.bin

usb3588: ddr/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin os3588.bin
	$(XROCK) maskrom ddr/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin os3588.bin --rc4-off

makeboot.out: src/makeboot.c
	$(CC) src/makeboot.c -o makeboot.out

# Bootable Pinebook Pro SPI image
pbp.img: makeboot.out
	./makeboot.out
	echo "Burn with: sudo dd if=pine.img of=/dev/sda bs=4M conv=fsync"

3399_OBJ := src/boot.o src/mmu.o src/asm.o src/main.o src/uart.o src/rk3399/timer.o src/vectors.o src/io.o src/rk3399/gpio.o src/rk3399/edp.o
3399_OBJ += src/rk3399/clock.o src/rk3399/soc.o src/lib.o src/bmp.o src/ohci.o src/gic.o src/i2c.o src/rk3399/mmc.o src/rk3399/io.o
#3399_OBJ += src/uboot.o
os3399.bin: $(3399_OBJ) Linker.ld
	$(ARMCC)-ld $(3399_OBJ) $(ARMLDFLAGS) -o src/boot.elf
	$(ARMCC)-objcopy -O binary src/boot.elf os3399.bin

3588_OBJ := src/boot.o src/main2.o src/rk3588/io.o src/uart.o src/asm.o src/vectors.o src/mmu.o src/lib.o
3588_OBJ += src/uboot.o
os3588.bin: $(3588_OBJ) Linker.ld
	$(ARMCC)-ld $(3588_OBJ) $(ARMLDFLAGS) -o src/boot.elf
	$(ARMCC)-objcopy -O binary src/boot.elf os3588.bin

src/uboot.c: /home/daniel/Pulled/coolpi-loader/u-boot.bin

src/ram2.o: ARMCFLAGS += -Os

DDR_OBJ := src/rk3399/sram.o src/rk3399/ddr.o src/io.o src/rk3399/io.o src/rk3399/gpio.o src/lib.o src/uart.o src/asm.o src/rk3399/clock.o src/rk3399/timer.o src/rk3399/ram2.o src/vectors.o
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
	sudo screen /dev/ttyUSB* 115200
	#sudo screen -L -Logfile log.txt /dev/ttyUSB* 115200

.PHONY: usb clean dmesg uart
