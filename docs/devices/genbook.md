# Genbook / Cool-pi notebook
Photos: https://www.flickr.com/photos/201609787@N08/albums/72177720322040367/

## Bare-metal firmware status

For a standalone SD or MaskROM test, use `genbook_demo.img` or
`demo_genbook.bin`. These contain the example EL2 payload. `genbook.img` and
`genbook.bin` contain firmware only: they expect a compatible FUEFI payload to
be appended and otherwise report `Bad payload magic` and halt.

In this flow, RK3588 BootROM reads the RKNS image from SD and places it in RAM.
The current bare-metal firmware does not contain an RK3588 SD/MMC driver and
cannot read files or partitions from the card after entry.

The minimal RK3588 DTS lists SDMMC and USB controller placeholders for
reference and future payload work, but listing a node does not initialize the
controller. Several required `reg` and `status` properties are absent, and the
current RK3588 firmware links no MMC, OHCI, xHCI, or USB-PHY driver. As a
result, the Genbook demo validates UART, eDP/framebuffer, DTB presence, memory
map, and EL2 handoff, but its keyboard prompt cannot receive USB input. Do not
use the minimal firmware DTB as a Linux board DTB. See
[Firmware images, payloads, and device trees](../payloads.md).

## BootROM and legacy recovery notes

The Genbook comes with U-boot SPL on the SPI flash, you can make it unbootable like so:

> **Warning:** the commands below intentionally corrupt or erase boot media.
> They are historical recovery notes, not guarded installers. Make verified
> off-device backups first and confirm that forced MaskROM entry works.

```
printf '\x00\x00\x00\x00' | dd of=/dev/mtdblock0 bs=1 seek=$((0x10000)) count=4 conv=notrunc
printf '\x00\x00\x00\x00' | dd of=/dev/mtdblock0 bs=1 seek=$((0x60000)) count=4 conv=notrunc
```
The boot image in SPI flash must have two copies (at `0x10000` and `0x60000`) so we need to cripple both
or the bootrom will load the unmodified copy.

The bootrom maskrom mode exposes itself on the left side USB-C port.

To boot back into u-boot:
```
xrock maskrom rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin u-boot-genbook.bin --rc4-off
```

The 'loader key' button on the bottom will disable all boot methods (SPI, emmc) and get the bootrom into maskrom mode.

In order to erase the emmc from maskrom mode:
```
xrock maskrom rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin rk3588_usbplug_v1.11.bin --rc4-off
xrock flash erase 0 100000
```
in order to erase the SPI:
```
xrock maskrom rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin rk3588_usbplug_v1.11.bin --rc4-off
```

There is currently no board-qualified, backup-and-verify SPI installer for the
Genbook in this repository. Do not substitute an unrelated file or perform a
whole-device overwrite merely to invalidate SPI; add a guarded RK3588 storage
policy and tested restoration path first.
