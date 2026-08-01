# Firmware images, payloads, and device trees

This repository produces several files that look bootable but have different
roles. The distinction matters most when choosing an SD image.

## The three execution models

| Model | Standalone? | Final program | Runtime storage drivers |
| --- | --- | --- | --- |
| Firmware-only `.bin` or `.img` | No | A separately appended FUEFI payload | No |
| Demo `.bin` or `.img` | Yes | The repository's example EL2 payload | No |
| Board-enabled RK3568 chainloader | Yes | BL31 and U-Boot at EL2 | U-Boot provides them |

The normal bare-metal firmware is a hardware-initialization and service layer,
not an operating system or conventional storage bootloader. It initializes the
supported parts of the board, exposes FUEFI calls, and unconditionally looks
for a `FuPayloadHeader` immediately after the firmware image. A firmware-only
artifact has no such payload: when run by itself it reports `Bad payload magic`
and halts. It is distributed as a base for payload developers.

The demo artifact is the corresponding firmware with `demo.bin` appended. It
is the correct standalone image for initial hardware testing. The demo reports
the board, current exception level, video mode, DTB status, and memory map. It
also exercises input on boards where a USB keyboard driver is implemented.

The optional RK3568 chainloader is a separate build variant. It does not use
the FUEFI payload path and deliberately excludes display, USB HID, and demo
code. It validates a U-Boot FIT and enters BL31, which then starts U-Boot.

## What SD boot means

An SD image does not imply that the bare-metal firmware contains an SD/MMC
driver. For the normal and demo flows, the immutable Rockchip BootROM reads the
RKNS image from SD before this project's code starts:

```text
SD card
  -> Rockchip BootROM reads the RKNS image
  -> BootROM runs the image's DDR loader
  -> BootROM loads the firmware or firmware+payload into RAM
  -> this project's firmware initializes the supported hardware
  -> jump to the appended payload, if present
```

Once control reaches this firmware, the SD card is only the delivery medium.
The normal/demo firmware cannot mount its partitions or load another file from
it. The same principle applies to direct MaskROM USB loading: USB transports
the image into RAM, but it does not automatically provide a runtime USB host
stack.

## Build a custom payload image

A FUEFI payload begins with the packed header described in `src/firmware.h`.
The supplied demo requests relocation to `0x00a00000` and runs at EL2. A custom
payload must use the selected board's reviewed memory map and must not overlap
the firmware, stacks, shared/DMA arena, framebuffer, or MMIO.

The basic source-tree workflow is:

```sh
# Build the board firmware and your FUEFI-compatible payload first.
cat rock3a.bin my_payload.bin > my_rock3a.bin

# Package the combined binary, not the firmware-only binary.
./makeboot.out --v2 \
  --ddr img/rk3568_ddr_1560MHz_v1.25.bin \
  --os my_rock3a.bin -o my_rock3a.img
```

Use the DDR loader and RKNS version appropriate for the selected board. The
existing `demo_<board>.bin` and demo-image rules are the reference packaging
implementation.

## What a device tree does—and does not do

A DTB is data passed to a payload. It can describe addresses, interrupts,
GPIOs, regulators, clocks, and board identity, but it does not initialize a
controller and it does not add a driver to the firmware. Rockchip BootROM does
not read this project's DTB.

There are therefore three separate questions for every device:

1. Is it described accurately in the selected board DTB?
2. Does this bare-metal firmware initialize it and expose a service for it?
3. Does the eventual payload or operating system contain a compatible driver?

All three are required before a payload can rely on a DT-described peripheral.
The minimal DTBs in this repository document the firmware contract; they are
not replacements for the complete upstream Linux board DTBs.

The distinction is especially visible on RK3588. Its minimal SoC DTS lists
SDMMC and several USB controller placeholders, but several `reg` and `status`
properties are intentionally absent or commented out, and the current RK3588
firmware links no MMC, OHCI, xHCI, or USB-PHY driver. Those nodes do not provide
runtime SD or USB support. The Genbook demo is useful for UART, eDP/framebuffer,
DTB, memory-map, and EL2 validation; its keyboard prompt cannot currently
receive USB input.

RK356x normal firmware has a broader implemented service set: HDMI/EDID,
framebuffer, polled OHCI boot-keyboard input, DTB, memory-map services, and
reset-to-MaskROM. It still has no filesystem or runtime SD/eMMC/NVMe driver.
See [RK356x common bare-metal firmware](rk356x/bare-metal.md) for its exact
memory, display, and input contract.

## Which image should I choose?

- Use a demo `.img` for a standalone first boot and hardware smoke test.
- Use a firmware-only `.bin` as the base for a custom appended payload.
- Repackage the combined firmware and payload before writing it to SD.
- Use a board-enabled chainloader when the goal is U-Boot, Linux, extlinux, or
  an EFI application from NVMe, SD, USB mass storage, or eMMC.
- Use a complete Linux board DTB when booting Linux; do not substitute one of
  the minimal firmware DTBs.
