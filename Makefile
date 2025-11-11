# Needed to compile targets of different architectures
convert_target_arm64 = $(patsubst %.o,%.arm64.o,$1)

XROCK ?= xrock
ARMCC ?= aarch64-linux-gnu

all: makeboot.out rock.out pinebook.bin pinebook-ddr.bin opi5.bin genbook.bin genbook-ddr.bin

ARMCFLAGS := -march=armv8-a -nostdlib -Wall -Wno-array-bounds -Isrc -Isrc/rk3399 -Isrc/rk3588 -ffunction-sections
ARMLDFLAGS := -T Linker.ld --gc-sections
# Align+pad to _end_of_image defined in linker script
OBJCOPYFLAGS := --pad-to 0x`readelf -s src/boot.elf | awk '/_end_of_image/ {print $$2}'`

PINEBOOK_DDR_OBJ := src/rk3399/ddr_shim.o src/rk3399/pinebook-ddr.o src/rk3399/io.o src/rk3399/gpio.o src/rk3399/timer.o src/lib.o src/pl011.o src/asm.o src/rk3399/clock.o src/rk3399/ddr-4gb-lpddr4.o src/vectors.o
PINEBOOK_DDR_OBJ := $(call convert_target_arm64,$(PINEBOOK_DDR_OBJ))

PINEBOOK_POC_DDR_OBJ := src/rk3399/ddr.o src/rk3399/pinebook-ddr.o src/rk3399/io.o src/rk3399/gpio.o src/rk3399/timer.o src/lib.o src/pl011.o src/asm.o src/rk3399/clock.o src/rk3399/ddr-4gb-lpddr4.o src/vectors.o
PINEBOOK_POC_DDR_OBJ := $(call convert_target_arm64,$(PINEBOOK_POC_DDR_OBJ))

GENBOOK_DDR_OBJ := $(call convert_target_arm64,src/rk3588/ddr.o src/rk3588/genbook-ddr.o src/rk3588/gpio.o src/rk3588/pwm.o src/lib.o)

3399_OBJ := src/boot.o src/mmu.o src/rk3399/ttbl.o src/asm.o src/pl011.o src/vectors.o src/rk3399/gpio.o src/rk3399/timer.o src/analogix_edp.o src/rk3399/vop.o src/firmware.o
3399_OBJ += src/rk3399/clock.o src/rk3399/soc.o src/lib.o src/ohci.o src/rk3399/mmc.o src/rk3399/io.o

PINEBOOK_OBJ := $(3399_OBJ) src/pinebook.o
PINEBOOK_OBJ := $(call convert_target_arm64,$(PINEBOOK_OBJ))

3588_OBJ := src/boot.o src/rk3588/io.o src/rk3588/sgrf.o src/rk3588/ioc.o src/rk3588/pmu.o src/rk3588/cru.o src/rk3588/vop2.o src/rk3588/video.o src/rk3588/gpio.o src/rk3588/pwm.o src/rk3588/tt.o
3588_OBJ += src/pl011.o src/asm.o src/vectors.o src/mmu.o src/lib.o src/firmware.o src/analogix_edp.o
3588_OBJ += external/samsung_phy_edp.o

OPI5_OBJ := $(3588_OBJ) src/opi5.o
OPI5_OBJ := $(call convert_target_arm64,$(OPI5_OBJ))

GENBOOK_OBJ := $(3588_OBJ) src/genbook.o
GENBOOK_OBJ := $(call convert_target_arm64,$(GENBOOK_OBJ))

$(call convert_target_arm64,src/rk3399/ram2.o): ARMCFLAGS += -Os
pinebook-ddr.bin: $(PINEBOOK_DDR_OBJ)
	$(ARMCC)-ld $(PINEBOOK_DDR_OBJ) -Ttext=0xFF8C2000 --gc-sections -o src/ddr.elf
	$(ARMCC)-objcopy -O binary src/ddr.elf pinebook-ddr.bin

pinebook-poc-ddr.bin: $(PINEBOOK_POC_DDR_OBJ)
	$(ARMCC)-ld $(PINEBOOK_POC_DDR_OBJ) -Ttext=0xFF8C2000 --gc-sections -o src/ddr.elf
	$(ARMCC)-objcopy -O binary src/ddr.elf pinebook-poc-ddr.bin

genbook-ddr.bin: $(GENBOOK_DDR_OBJ)
	$(ARMCC)-ld $(GENBOOK_DDR_OBJ) -Ttext=0xFF001000 --gc-sections -o src/temp.elf
	$(ARMCC)-objcopy -O binary src/temp.elf genbook-ddr.bin

pinebook.bin: $(PINEBOOK_OBJ) Linker.ld
	$(ARMCC)-ld $(PINEBOOK_OBJ) $(ARMLDFLAGS) -o src/boot.elf
	$(ARMCC)-objcopy $(OBJCOPYFLAGS) -O binary src/boot.elf pinebook.bin

pinebook.img: makeboot.out pinebook-poc-ddr.bin demo_pinebook.bin
	./makeboot.out --v1 --ddr pinebook-poc-ddr.bin --os demo_pinebook.bin -o pinebook.img

opi5.bin: $(OPI5_OBJ) Linker.ld
	$(ARMCC)-ld $(OPI5_OBJ) $(ARMLDFLAGS) -o src/boot.elf
	$(ARMCC)-objcopy -O binary src/boot.elf opi5.bin

opi5.img: makeboot.out img/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin opi5.bin
	./makeboot.out --v2 --ddr img/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin --os opi5.bin -o opi5.img

genbook.img: makeboot.out genbook-ddr.bin demo_genbook.bin
	./makeboot.out --v2 --ddr genbook-ddr.bin --os demo_genbook.bin -o genbook.img

opi5_.img: img/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin opi5.bin
	mkimage -n rk3588 -T rksd -d img/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin:opi5.bin uboot.bin
	dd if=uboot.bin of=opi5_.img seek=64 conv=fsync

genbook.bin: $(GENBOOK_OBJ) Linker.ld
	$(ARMCC)-ld $(GENBOOK_OBJ) $(ARMLDFLAGS) -o src/boot.elf
	$(ARMCC)-objcopy $(OBJCOPYFLAGS) -O binary src/boot.elf genbook.bin

DEMO_OBJ := demo/entry.o demo/main.o demo/bmp.o
DEMO_OBJ := $(call convert_target_arm64,$(DEMO_OBJ))

demo_pinebook.bin: $(DEMO_OBJ) pinebook.bin
	$(ARMCC)-ld $(DEMO_OBJ) -Ttext=$(shell printf '0x%X' $$(stat -c%s pinebook.bin)) -o src/boot.elf
	$(ARMCC)-objcopy -O binary src/boot.elf demo_for_pinebook.bin
	cat pinebook.bin demo_for_pinebook.bin > demo_pinebook.bin

demo_genbook.bin: $(DEMO_OBJ) genbook.bin
	$(ARMCC)-ld $(DEMO_OBJ) -Ttext=$(shell printf '0x%X' $$(stat -c%s genbook.bin)) -o src/boot.elf
	$(ARMCC)-objcopy -O binary src/boot.elf demo_for_genbook.bin
	cat genbook.bin demo_for_genbook.bin > demo_genbook.bin

%.o: %.c
	gcc -MMD -c $< -o $@
%.arm64.o: %.c
	$(ARMCC)-gcc -MMD -c $< $(ARMCFLAGS) -o $@
%.arm64.o: %.S
	$(ARMCC)-gcc -D __ASM__ -MMD -c $< $(ARMCFLAGS) -o $@

-include $(wildcard **/*.d)

clean:
	find src demo tools \( -name '*.d' -o -name '*.o' -o -name '*.elf' -o -name '*.bin' \) -type f -delete
	rm -rf *.bin *.elf *.out *.img

usb3399: rock.out pinebook-poc-ddr.bin demo_pinebook.bin
	./rock.out --v1 --ddr pinebook-poc-ddr.bin --os demo_pinebook.bin

usb3588: rock.out genbook-ddr.bin demo_genbook.bin
	./rock.out --v2 --ddr genbook-ddr.bin --os demo_genbook.bin

makeboot.out: tools/makeboot.c
	$(CC) tools/makeboot.c -o makeboot.out

rock.out: tools/rock.c
	$(CC) tools/rock.c `pkg-config --cflags --libs libusb-1.0` -o rock.out

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

maskrom3588:
	xrock maskrom img/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin img/rk3588_usbplug_v1.11.bin --rc4-off

.PHONY: usb clean dmesg uart

-include config.mk
