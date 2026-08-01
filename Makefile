# Needed to compile targets of different architectures
convert_target_arm64 = $(patsubst %.o,%.arm64.o,$1)
convert_target_rk356x = $(patsubst %.o,%.rk356x.o,$1)

# Chainloader objects deliberately live in a board-and-variant namespace.
CHAINLOAD_BOARDS := yy3568 rock3a
CHAINLOAD_MANIFEST := tools/chainload-manifest.py
chain_c_obj = $(patsubst %.c,build/chainload/$(1)/obj/%.o,$(filter %.c,$(2)))
chain_s_obj = $(patsubst %.S,build/chainload/$(1)/obj/%.o,$(filter %.S,$(2)))
chain_get = $(shell python3 $(CHAINLOAD_MANIFEST) get $(1) $(2))

XROCK ?= xrock
RKDEVELOPTOOL ?= rkdeveloptool
ARMCC ?= aarch64-linux-gnu
DIST_DIR ?= dist

RELEASE_ARTIFACTS := pinebook.bin demo_pinebook.bin pinebook.img demo_pinebook.img
RELEASE_ARTIFACTS += pinebook-ddr.bin pinebook-poc-ddr.bin
RELEASE_ARTIFACTS += genbook.bin demo_genbook.bin genbook.img genbook_demo.img genbook-ddr.bin
RELEASE_ARTIFACTS += roc3566.bin demo_roc3566.bin roc3566.img demo_roc3566.img
RELEASE_ARTIFACTS += yy3568.bin demo_yy3568.bin yy3568.img demo_yy3568.img
RELEASE_ARTIFACTS += rock3a.bin demo_rock3a.bin rock3a.img demo_rock3a.img

all: makeboot.out rock.out pinebook.bin pinebook-ddr.bin opi5.bin genbook.bin genbook-ddr.bin genbook_demo.img demo_pinebook.img
all: pinebook.img genbook.img
all: roc3566.bin demo_roc3566.bin roc3566.img demo_roc3566.img
all: yy3568.bin demo_yy3568.bin yy3568.img demo_yy3568.img
all: rock3a.bin demo_rock3a.bin rock3a.img demo_rock3a.img

ARMCFLAGS := -march=armv8-a -nostdlib -Wall -Wno-array-bounds -Isrc -Isrc/rk3399 -Isrc/rk3588 -Isrc/rk356x -ffunction-sections -ffreestanding
ARMLDFLAGS := -T Linker.ld --gc-sections
# Align+pad to _end_of_image defined in linker script
OBJCOPYFLAGS = --pad-to 0x`readelf -s src/$@.elf | awk '/_end_of_image/ {print $$2}'`

PINEBOOK_DDR_OBJ := src/rk3399/ddr_shim.o src/rk3399/pinebook-ddr.o src/rk3399/io.o src/rk3399/gpio.o src/rk3399/timer.o src/lib.o src/pl011.o src/asm.o src/rk3399/clock.o src/rk3399/ddr-4gb-lpddr4.o src/vectors.o
PINEBOOK_DDR_OBJ := $(call convert_target_arm64,$(PINEBOOK_DDR_OBJ))

PINEBOOK_POC_DDR_OBJ := src/rk3399/ddr.o src/rk3399/pinebook-ddr.o src/rk3399/io.o src/rk3399/gpio.o src/rk3399/timer.o src/lib.o src/pl011.o src/asm.o src/rk3399/clock.o src/rk3399/ddr-4gb-lpddr4.o src/vectors.o
PINEBOOK_POC_DDR_OBJ := $(call convert_target_arm64,$(PINEBOOK_POC_DDR_OBJ))
$(call convert_target_arm64,src/rk3399/ram2.o): ARMCFLAGS += -Os

GENBOOK_DDR_OBJ := $(call convert_target_arm64,src/rk3588/ddr.o src/rk3588/genbook-ddr.o src/rk3588/gpio.o src/rk3588/pwm.o src/lib.o)

3399_OBJ := src/boot.o src/mmu.o src/asm.o src/pl011.o src/vectors.o src/rk3399/gpio.o src/rk3399/timer.o src/analogix_edp.o src/rk3399/vop.o src/firmware.o
3399_OBJ += src/rk3399/clock.o src/rk3399/soc.o src/lib.o src/ohci.o src/hid_keyboard.o src/input.o src/rk3399/mmc.o src/rk3399/io.o

PINEBOOK_OBJ := $(3399_OBJ) src/pinebook.o
PINEBOOK_OBJ := $(call convert_target_arm64,$(PINEBOOK_OBJ))
$(PINEBOOK_OBJ): src/rk3399/pinebook.dtb.out.h

3588_OBJ := src/boot.o src/rk3588/io.o src/rk3588/sgrf.o src/rk3588/ioc.o src/rk3588/pmu.o src/rk3588/cru.o src/rk3588/vop2.o src/rk3588/video.o src/rk3588/gpio.o src/rk3588/pwm.o
3588_OBJ += src/pl011.o src/asm.o src/vectors.o src/mmu.o src/lib.o src/firmware.o src/input.o src/analogix_edp.o
3588_OBJ += external/samsung_phy_edp.o

OPI5_OBJ := $(3588_OBJ) src/opi5.o
OPI5_OBJ := $(call convert_target_arm64,$(OPI5_OBJ))

GENBOOK_OBJ := $(3588_OBJ) src/genbook.o
GENBOOK_OBJ := $(call convert_target_arm64,$(GENBOOK_OBJ))
$(GENBOOK_OBJ): src/rk3588/genbook.dtb.out.h

RK356X_OBJ := src/boot.o src/mmu.o src/asm.o src/pl011.o src/vectors.o src/lib.o
RK356X_OBJ += src/firmware.o src/input.o src/hid_keyboard.o src/ohci.o src/edid.o
RK356X_OBJ += src/rk356x/board.o src/rk356x/io.o src/rk356x/dram.o src/rk356x/gpio.o src/rk356x/sgrf.o
RK356X_OBJ += src/rk356x/cru.o src/rk356x/vop2.o src/rk356x/hdmi.o src/rk356x/usb.o
RK356X_OBJ := $(call convert_target_rk356x,$(RK356X_OBJ))

ROC3566_OBJ := $(RK356X_OBJ) src/roc3566.rk356x.o
YY3568_OBJ := $(RK356X_OBJ) src/yy3568.rk356x.o
ROCK3A_OBJ := $(RK356X_OBJ) src/rock3a.rk356x.o
$(ROC3566_OBJ): src/rk356x/roc3566.dtb.out.h
$(YY3568_OBJ): src/rk356x/yy3568.dtb.out.h
$(ROCK3A_OBJ): src/rk356x/rock3a.dtb.out.h

CHAINLOAD_SRC := src/boot.S src/asm.S src/vectors.S src/mmu.c src/lib.c src/pl011.c
CHAINLOAD_SRC += src/chainload/compat.c src/chainload/sha256.c src/chainload/fit.c
CHAINLOAD_SRC += src/chainload/tf_a.c src/chainload/loader.c src/chainload/handoff.S
CHAINLOAD_SRC += src/chainload/rk3568.c
CHAINLOAD_SRC += external/libfdt/fdt.c external/libfdt/fdt_ro.c
CHAINLOAD_SRC += external/libfdt/fdt_rw.c external/libfdt/fdt_wip.c
CHAINLOAD_CFLAGS := $(ARMCFLAGS) -Isrc/chainload -Iexternal/libfdt -Os -fdata-sections
CHAINLOAD_CFLAGS += -fno-unwind-tables -fno-asynchronous-unwind-tables
CHAINLOAD_CFLAGS += -DSTACK_TOP=0x08000000

define CHAINLOAD_BOARD_VARIABLES
CHAINLOAD_OBJ_$(1) := $(call chain_c_obj,$(1),$(CHAINLOAD_SRC)) $(call chain_s_obj,$(1),$(CHAINLOAD_SRC))
CHAINLOAD_FIT_$(1) := $(call chain_get,$(1),artifacts.fit)
CHAINLOAD_BINARY_$(1) := $(call chain_get,$(1),artifacts.binary)
CHAINLOAD_IMAGE_$(1) := $(call chain_get,$(1),artifacts.image)
CHAINLOAD_IDBLOCK_$(1) := $(call chain_get,$(1),artifacts.idblock)
CHAINLOAD_SPI_$(1) := $(call chain_get,$(1),artifacts.spi_nor)
CHAINLOAD_SOURCE_$(1) := $(call chain_get,$(1),artifacts.source)
CHAINLOAD_DDR_$(1) := $(call chain_get,$(1),boot_media.ddr)
CHAINLOAD_BL31_$(1) := $(call chain_get,$(1),bl31.path)
CHAINLOAD_MEDIA_ARTIFACTS_$(1) := $$(CHAINLOAD_IMAGE_$(1)) $$(CHAINLOAD_IDBLOCK_$(1)) $$(CHAINLOAD_SPI_$(1))
endef

$(foreach board,$(CHAINLOAD_BOARDS),$(eval $(call CHAINLOAD_BOARD_VARIABLES,$(board))))

DEMO_OBJ := demo/entry.o demo/main.o demo/bmp.o demo/vectors.o
DEMO_OBJ := $(call convert_target_arm64,$(DEMO_OBJ))

pinebook-ddr.bin: $(PINEBOOK_DDR_OBJ)
	$(ARMCC)-ld $(PINEBOOK_DDR_OBJ) -Ttext=0xFF8C2000 --gc-sections -o src/$@.elf
	$(ARMCC)-objcopy -O binary src/$@.elf pinebook-ddr.bin

pinebook-poc-ddr.bin: $(PINEBOOK_POC_DDR_OBJ)
	$(ARMCC)-ld $(PINEBOOK_POC_DDR_OBJ) -Ttext=0xFF8C2000 --gc-sections -o src/$@.elf
	$(ARMCC)-objcopy -O binary src/$@.elf pinebook-poc-ddr.bin

genbook-ddr.bin: $(GENBOOK_DDR_OBJ)
	$(ARMCC)-ld $(GENBOOK_DDR_OBJ) -Ttext=0xFF001000 --gc-sections -o src/$@.elf
	$(ARMCC)-objcopy -O binary src/$@.elf genbook-ddr.bin

pinebook.bin: $(PINEBOOK_OBJ) Linker.ld
	$(ARMCC)-ld $(PINEBOOK_OBJ) $(ARMLDFLAGS) -o src/$@.elf
	$(ARMCC)-objcopy $(OBJCOPYFLAGS) -O binary src/$@.elf pinebook.bin

pinebook.img: makeboot.out pinebook-poc-ddr.bin pinebook.bin
	./makeboot.out --v1 --ddr pinebook-poc-ddr.bin --os pinebook.bin -o pinebook.img

opi5.bin: $(OPI5_OBJ) Linker.ld
	$(ARMCC)-ld $(OPI5_OBJ) $(ARMLDFLAGS) -o src/$@.elf
	$(ARMCC)-objcopy -O binary src/$@.elf opi5.bin

opi5.img: makeboot.out img/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin opi5.bin
	./makeboot.out --v2 --ddr img/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin --os opi5.bin -o opi5.img

genbook.img: makeboot.out genbook-ddr.bin genbook.bin
	./makeboot.out --v2 --ddr genbook-ddr.bin --os genbook.bin -o genbook.img

genbook_demo.img: makeboot.out genbook-ddr.bin demo_genbook.bin
	./makeboot.out --v2 --ddr genbook-ddr.bin --os demo_genbook.bin -o genbook_demo.img

demo_pinebook.img: makeboot.out pinebook-poc-ddr.bin demo_pinebook.bin
	./makeboot.out --v1 --ddr pinebook-poc-ddr.bin --os demo_pinebook.bin -o demo_pinebook.img

genbook.bin: $(GENBOOK_OBJ) Linker.ld
	$(ARMCC)-ld $(GENBOOK_OBJ) $(ARMLDFLAGS) -o src/$@.elf
	$(ARMCC)-objcopy $(OBJCOPYFLAGS) -O binary src/$@.elf genbook.bin

roc3566.bin: $(ROC3566_OBJ) Linker.ld
	$(ARMCC)-ld $(ROC3566_OBJ) $(ARMLDFLAGS) -o src/$@.elf
	$(ARMCC)-objcopy $(OBJCOPYFLAGS) -O binary src/$@.elf $@

yy3568.bin: $(YY3568_OBJ) Linker.ld
	$(ARMCC)-ld $(YY3568_OBJ) $(ARMLDFLAGS) -o src/$@.elf
	$(ARMCC)-objcopy $(OBJCOPYFLAGS) -O binary src/$@.elf $@

rock3a.bin: $(ROCK3A_OBJ) Linker.ld
	$(ARMCC)-ld $(ROCK3A_OBJ) $(ARMLDFLAGS) -o src/$@.elf
	$(ARMCC)-objcopy $(OBJCOPYFLAGS) -O binary src/$@.elf $@

demo.bin: $(DEMO_OBJ)
	$(ARMCC)-ld $(DEMO_OBJ) -Ttext=0xa00000 -o src/$@.elf
	$(ARMCC)-objcopy --gap-fill 0 --pad-to 0x`readelf -s src/$@.elf | awk '$$8 == "_end" {print $$2}'` -O binary src/$@.elf demo.bin

demo_pinebook.bin: demo.bin pinebook.bin
	cat pinebook.bin demo.bin > demo_pinebook.bin

demo_genbook.bin: demo.bin genbook.bin
	cat genbook.bin demo.bin > demo_genbook.bin

demo_roc3566.bin: demo.bin roc3566.bin
	cat roc3566.bin demo.bin > $@

demo_yy3568.bin: demo.bin yy3568.bin
	cat yy3568.bin demo.bin > $@

demo_rock3a.bin: demo.bin rock3a.bin
	cat rock3a.bin demo.bin > $@

roc3566.img: makeboot.out img/rk3566_ddr_1056MHz_v1.25.bin roc3566.bin
	./makeboot.out --v2 --ddr img/rk3566_ddr_1056MHz_v1.25.bin --os roc3566.bin -o $@

demo_roc3566.img: makeboot.out img/rk3566_ddr_1056MHz_v1.25.bin demo_roc3566.bin
	./makeboot.out --v2 --ddr img/rk3566_ddr_1056MHz_v1.25.bin --os demo_roc3566.bin -o $@

yy3568.img: makeboot.out img/rk3568_ddr_1560MHz_v1.25.bin yy3568.bin
	./makeboot.out --v2 --ddr img/rk3568_ddr_1560MHz_v1.25.bin --os yy3568.bin -o $@

demo_yy3568.img: makeboot.out img/rk3568_ddr_1560MHz_v1.25.bin demo_yy3568.bin
	./makeboot.out --v2 --ddr img/rk3568_ddr_1560MHz_v1.25.bin --os demo_yy3568.bin -o $@

rock3a.img: makeboot.out img/rk3568_ddr_1560MHz_v1.25.bin rock3a.bin
	./makeboot.out --v2 --ddr img/rk3568_ddr_1560MHz_v1.25.bin --os rock3a.bin -o $@

demo_rock3a.img: makeboot.out img/rk3568_ddr_1560MHz_v1.25.bin demo_rock3a.bin
	./makeboot.out --v2 --ddr img/rk3568_ddr_1560MHz_v1.25.bin --os demo_rock3a.bin -o $@

define CHAINLOAD_BOARD_RULES
build/chainload/$(1)/generated/board_config.h: config/chainload/$(1).json $(CHAINLOAD_MANIFEST)
	@mkdir -p $$(@D)
	python3 $(CHAINLOAD_MANIFEST) generate-header $(1) $$@

$$(CHAINLOAD_OBJ_$(1)): build/chainload/$(1)/generated/board_config.h

build/chainload/$(1)/obj/%.o: %.c
	@mkdir -p $$(@D)
	$$(ARMCC)-gcc -MMD -c $$< $$(CHAINLOAD_CFLAGS) -Ibuild/chainload/$(1)/generated -o $$@

build/chainload/$(1)/obj/%.o: %.S
	@mkdir -p $$(@D)
	$$(ARMCC)-gcc -D__ASM__ -MMD -c $$< $$(CHAINLOAD_CFLAGS) -Ibuild/chainload/$(1)/generated -o $$@

build/chainload/$(1)/stage.elf: $$(CHAINLOAD_OBJ_$(1)) Chainload-rk356x.ld
	@mkdir -p $$(@D)
	$$(ARMCC)-ld $$(CHAINLOAD_OBJ_$(1)) -T Chainload-rk356x.ld --gc-sections -o $$@

build/chainload/$(1)/stage.bin: build/chainload/$(1)/stage.elf
	$$(ARMCC)-objcopy --gap-fill 0 --pad-to 0x`$$(ARMCC)-readelf -s $$< | awk '$$$$8 == "_end_of_image" {print $$$$2}'` -O binary $$< $$@

build/chainload/$(1)/.uboot-built: config/chainload/$(1).json \
		$$(shell find config/chainload/$(1) -type f) $$(CHAINLOAD_BL31_$(1)) \
		tools/build-chainload-uboot.sh $(CHAINLOAD_MANIFEST)
	@mkdir -p $$(@D)
	bash tools/build-chainload-uboot.sh $(1)
	@touch $$@

$$(CHAINLOAD_FIT_$(1)) $$(CHAINLOAD_SOURCE_$(1)): build/chainload/$(1)/.uboot-built
	@test -f $$@ || { echo "$$@ was not generated" >&2; exit 2; }

$$(CHAINLOAD_BINARY_$(1)): build/chainload/$(1)/stage.bin $$(CHAINLOAD_FIT_$(1))
	cat $$^ > $$@

build/chainload/$(1)/.media-built: $$(CHAINLOAD_BINARY_$(1)) $$(CHAINLOAD_DDR_$(1)) \
		config/chainload/$(1).json tools/build-chainload-media.sh $(CHAINLOAD_MANIFEST)
	bash tools/build-chainload-media.sh $(1)
	@touch $$@

$$(CHAINLOAD_MEDIA_ARTIFACTS_$(1)): build/chainload/$(1)/.media-built
	@if ! test -f $$@; then \
		rm -f build/chainload/$(1)/.media-built; \
		$$(MAKE) build/chainload/$(1)/.media-built; \
	fi
	@test -f $$@ || { echo "$$@ was not generated" >&2; exit 2; }
endef

$(foreach board,$(CHAINLOAD_BOARDS),$(eval $(call CHAINLOAD_BOARD_RULES,$(board))))

tools/chainfit.out: tools/chainfit.c src/chainload/fit.c src/chainload/sha256.c \
		src/chainload/compat.c \
		external/libfdt/fdt.c external/libfdt/fdt_ro.c external/libfdt/fdt_rw.c \
		external/libfdt/fdt_wip.c
	$(CC) -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra \
		-Isrc -Isrc/chainload -Iexternal/libfdt $^ -o $@

chainload:
	@test -n "$(BOARD)" || { echo "BOARD is required" >&2; exit 2; }
	@python3 $(CHAINLOAD_MANIFEST) validate "$(BOARD)"
	@artifacts="$$(python3 $(CHAINLOAD_MANIFEST) get "$(BOARD)" artifacts.binary) $$(python3 $(CHAINLOAD_MANIFEST) get "$(BOARD)" artifacts.image) $$(python3 $(CHAINLOAD_MANIFEST) get "$(BOARD)" artifacts.idblock) $$(python3 $(CHAINLOAD_MANIFEST) get "$(BOARD)" artifacts.spi_nor)"; \
	$(MAKE) $$artifacts

chainload-media:
	@test -n "$(BOARD)" || { echo "BOARD is required" >&2; exit 2; }
	@test -n "$(MEDIA)" || { echo "MEDIA is required" >&2; exit 2; }
	@python3 $(CHAINLOAD_MANIFEST) validate "$(BOARD)"
	@artifact=$$(python3 $(CHAINLOAD_MANIFEST) get "$(BOARD)" "boot_media.$(MEDIA).artifact" 2>/dev/null) || { echo "board '$(BOARD)' does not support media '$(MEDIA)'" >&2; exit 2; }; \
	$(MAKE) "$$artifact"

flash-chainload:
	@test -n "$(BOARD)" -a -n "$(MEDIA)" -a -n "$(BACKUP_DIR)" -a -n "$(CONFIRM)" || \
		{ echo "BOARD, MEDIA, BACKUP_DIR, and CONFIRM are required" >&2; exit 2; }
	$(MAKE) chainload-media BOARD="$(BOARD)" MEDIA="$(MEDIA)"
	XROCK="$(XROCK)" RKDEVELOPTOOL="$(RKDEVELOPTOOL)" bash tools/flash-chainload.sh \
		flash "$(BOARD)" "$(MEDIA)" "$(BACKUP_DIR)" "$(CONFIRM)"

restore-chainload:
	@test -n "$(BACKUP)" -a -n "$(CONFIRM)" || \
		{ echo "BACKUP and CONFIRM are required" >&2; exit 2; }
	XROCK="$(XROCK)" RKDEVELOPTOOL="$(RKDEVELOPTOOL)" bash tools/flash-chainload.sh \
		restore "$(BACKUP)" "$(CONFIRM)"

chainload-check:
	@test -n "$(BOARD)" || { echo "BOARD is required" >&2; exit 2; }
	@python3 $(CHAINLOAD_MANIFEST) validate "$(BOARD)"
	@binary=$$(python3 $(CHAINLOAD_MANIFEST) get "$(BOARD)" artifacts.binary); \
	$(MAKE) "$$binary" tools/chainfit.out
	@if test -z "$(UBOOT_ITB)"; then \
		media="$$(python3 $(CHAINLOAD_MANIFEST) get "$(BOARD)" artifacts.image) $$(python3 $(CHAINLOAD_MANIFEST) get "$(BOARD)" artifacts.idblock) $$(python3 $(CHAINLOAD_MANIFEST) get "$(BOARD)" artifacts.spi_nor)"; \
		$(MAKE) $$media; \
	fi
	@fit=$$(python3 $(CHAINLOAD_MANIFEST) get "$(BOARD)" artifacts.fit); \
	args=$$(python3 $(CHAINLOAD_MANIFEST) chainfit-args "$(BOARD)"); \
	./tools/chainfit.out "$(BOARD)" "$$fit" $$args
	UBOOT_ITB="$(UBOOT_ITB)" python3 tests/check_chainload.py --board "$(BOARD)"

makeboot.out: tools/makeboot.o
	$(CC) tools/makeboot.o -o makeboot.out

rock.out: tools/rock.o
	$(CC) tools/rock.o `pkg-config --cflags --libs libusb-1.0` -o rock.out

%.o: %.c
	gcc -MMD -c $< -o $@
%.arm64.o: %.c
	$(ARMCC)-gcc -MMD -c $< $(ARMCFLAGS) -o $@
%.arm64.o: %.S
	$(ARMCC)-gcc -D __ASM__ -MMD -c $< $(ARMCFLAGS) -o $@
%.rk356x.o: %.c
	$(ARMCC)-gcc -MMD -c $< $(ARMCFLAGS) -DSTACK_TOP=0x08000000 -o $@
%.rk356x.o: %.S
	$(ARMCC)-gcc -D __ASM__ -MMD -c $< $(ARMCFLAGS) -DSTACK_TOP=0x08000000 -o $@

%.dtb.out.h: %.dts
	set -o pipefail; cpp -nostdinc -undef -x assembler-with-cpp $< | dtc | xxd -i -n dtb_data > $@

-include $(wildcard **/*.d)

clean:
	find src demo tools tests \( -name '*.d' -o -name '*.o' -o -name '*.elf' -o -name '*.bin' -o -name '*.out.h' -o -name '*.out' \) -type f -delete
	rm -rf *.bin *.elf *.out *.img *.d *.itb *-u-boot-source.tar.xz build/chainload

usb3399: rock.out pinebook-poc-ddr.bin demo_pinebook.bin
	./rock.out --v1 --ddr pinebook-poc-ddr.bin --os demo_pinebook.bin

usb3588: rock.out genbook-ddr.bin demo_genbook.bin
	./rock.out --v2 --ddr genbook-ddr.bin --os demo_genbook.bin

usb3566: rock.out img/rk3566_ddr_1056MHz_v1.25.bin demo_roc3566.bin
	./rock.out --v2 --ddr img/rk3566_ddr_1056MHz_v1.25.bin --os demo_roc3566.bin

usb:
	@test -n "$(BOARD)" || { echo "BOARD is required" >&2; exit 2; }
	@case "$(BOARD)" in \
	roc3566) $(MAKE) usb3566 ;; \
	yy3568) $(MAKE) rock.out img/rk3568_ddr_1560MHz_v1.25.bin demo_yy3568.bin && \
		./rock.out --v2 --ddr img/rk3568_ddr_1560MHz_v1.25.bin --os demo_yy3568.bin ;; \
	rock3a) $(MAKE) rock.out img/rk3568_ddr_1560MHz_v1.25.bin demo_rock3a.bin && \
		./rock.out --v2 --ddr img/rk3568_ddr_1560MHz_v1.25.bin --os demo_rock3a.bin ;; \
	*) echo "board '$(BOARD)' has no RK356x USB target" >&2; exit 2 ;; \
	esac

usb-chainload:
	@test -n "$(BOARD)" || { echo "BOARD is required" >&2; exit 2; }
	@python3 $(CHAINLOAD_MANIFEST) validate "$(BOARD)"
	@ddr=$$(python3 $(CHAINLOAD_MANIFEST) get "$(BOARD)" boot_media.ddr); \
	binary=$$(python3 $(CHAINLOAD_MANIFEST) get "$(BOARD)" artifacts.binary); \
	$(MAKE) rock.out "$$ddr" "$$binary" && ./rock.out --v2 --ddr "$$ddr" --os "$$binary"

usb3568:
	@echo "warning: usb3568 is a deprecated alias for 'make usb BOARD=yy3568'" >&2
	@$(MAKE) usb BOARD=yy3568

usb3568-uboot:
	@echo "warning: usb3568-uboot is a deprecated alias for 'make usb-chainload BOARD=yy3568'" >&2
	@$(MAKE) usb-chainload BOARD=yy3568

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

maskrom3566:
	$(XROCK) maskrom img/rk3566_ddr_1056MHz_v1.25.bin img/rk356x_usbplug_v1.17.bin --rc4-off

maskrom3568:
	$(XROCK) maskrom img/rk3568_ddr_1560MHz_v1.25.bin img/rk356x_usbplug_v1.17.bin --rc4-off

check: all build/chainload/yy3568/stage.bin build/chainload/rock3a/stage.bin
	$(CC) -std=c11 -Wall -Wextra -ffunction-sections -fdata-sections -Isrc \
		tests/unit.c tests/host_stubs.c src/edid.c src/input.c src/hid_keyboard.c src/ohci.c \
		-Wl,--gc-sections -o tests/unit.out
	./tests/unit.out
	$(CC) -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra \
		-Isrc -Isrc/chainload -Iexternal/libfdt \
		tests/chainload_unit.c src/chainload/fit.c src/chainload/sha256.c src/chainload/tf_a.c \
		src/chainload/loader.c src/chainload/compat.c \
		external/libfdt/fdt.c external/libfdt/fdt_ro.c external/libfdt/fdt_rw.c \
		external/libfdt/fdt_wip.c external/libfdt/fdt_sw.c external/libfdt/fdt_empty_tree.c \
		-o tests/chainload-unit.out
	./tests/chainload-unit.out
	python3 tests/check.py
	python3 tests/check_chainload.py --offline
	python3 tests/check_chainload_flash.py
	python3 tests/check_release.py

release-dist: $(RELEASE_ARTIFACTS)
	@test -n "$(VERSION)" || { echo "VERSION is required (vMAJOR.MINOR.PATCH)" >&2; exit 2; }
	bash tools/release-dist.sh "$(VERSION)" "$(DIST_DIR)"

.PHONY: all check release-dist clean chainload chainload-media chainload-check flash-chainload restore-chainload usb usb-chainload usb3399 usb3588 usb3566 usb3568 usb3568-uboot dmesg uart uart2 bear maskrom3588 maskrom3566 maskrom3568

-include $(foreach board,$(CHAINLOAD_BOARDS),$(CHAINLOAD_OBJ_$(board):.o=.d))

-include config.mk
